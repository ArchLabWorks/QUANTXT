/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <string.h>

#include "state.h"
#include "system.h"
#include "modules.h"
#include "util.h"
#include "engine.h"



/* ---------------------------------------------------------
 * Initialize default model parameters
 * --------------------------------------------------------- */
void init_default_params(Params *p)
{
    p->ai_coef = 0.3;

    p->m1_w  = 0.15;
    p->m2_w  = 0.10;
    p->m3_w  = 0.10;
    p->m4_w  = 0.10;
    p->m5_w  = 0.10;
    p->m6_w  = 0.10;
    p->m7_w  = 0.10;
    p->m10_w = 0.10;
    p->m13_w = 0.15;

    p->hysteresis          = 0.82;
    p->terminal_persistence = 0.7;
    p->raw_c               = 1.0;
}

/* ---------------------------------------------------------
 * SYSTEM_engine — main nonlinear macro‑risk engine
 * --------------------------------------------------------- */
void SYSTEM_engine(
    const State *s,
    const Params *p,
    double prev_M8,
    int has_prev,
    int reflexive_mode,
    int periods_above,
    SystemOut *o
) {
    /* Ensure all fields are initialized */
    memset(o, 0, sizeof(SystemOut));

    /*
     * Correct module evaluation order:
     *
     * Pass 1 — independent modules:
     *   M3, M4, M5, M7, M9, M10, M13
     *
     * Pass 2 — dependent modules:
     *   M1 (needs M3)
     *   M2 (needs M5)
     *   M6 (needs M9)
     */

    /* Pass 1 */
    o->M3  = M3(s->xdate, s->cbo_deficit);
    o->M4  = M4(s->usd_reserve_share);
    o->M5  = M5(s->ofr, s->hy_spread);
    o->M7  = M7(s->dxy_mom, s->oil_price);
    o->M9  = M9(s->ai_capex);
    o->M10 = M10(s->geopolitical_risk);
    o->M13 = M13(s->investor_sentiment);

    /* Pass 2 */
    o->M1 = M1(s->int_rev, o->M3);
    o->M2 = M2(s->tail_risk, s->liq_gap, o->M5);
    o->M6 = M6(s->sahm, o->M9, s->lagged_ai);

    /* Weighted stress index */
    o->raw_stress =
        p->m1_w  * o->M1  + p->m2_w  * o->M2  + p->m3_w  * o->M3  +
        p->m4_w  * o->M4  + p->m5_w  * o->M5  + p->m6_w  * o->M6  +
        p->m7_w  * o->M7  + p->m10_w * o->M10 + p->m13_w * o->M13;

    /* AI damping (M9) */
    o->ai_damping = 1.0 - p->ai_coef * o->M9;
    if (o->ai_damping < 0.0) o->ai_damping = 0.0;

    /* Composite stress score */
    o->M8 = clip01(o->raw_stress * o->ai_damping);

    /* EMA smoothing */
    if (has_prev) {
        o->M8 = p->terminal_persistence * prev_M8 +
                (1.0 - p->terminal_persistence) * o->M8;
    }

    /* Regime classification */
    if (o->M8 > p->hysteresis)
        strcpy(o->regime, "High Risk");
    else
        strcpy(o->regime, "Normal");

    /* Derived outputs */
    o->ponr_probability  = clip01(o->M8 - 0.7);
    o->stress_dispersion = 0.1;
    o->debt_pressure     = o->M1 + o->M3;
}
