/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <string.h>

#include "state.h"
#include "system.h"
#include "modules.h"
#include "util.h"
#include "engine.h"
#include "calibpar.h"

/* ---------------------------------------------------------
 * Initialize default model parameters
 * (Legacy nonlinear engine only — NOT used by OLS engine)
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

    p->hysteresis           = 0.82;
    p->terminal_persistence = 0.7;
    p->raw_c                = 1.0;
}

/* ---------------------------------------------------------
 * compute_system_out — calibrated OLS-engine shim
 *
 * NEW SIGNATURE:
 *     compute_system_out(const State *s,
 *                        const CalibParams *p,
 *                        SystemOut *o)
 *
 * This is the ONLY scoring path used by MAIN.C and DASH.C.
 * --------------------------------------------------------- */
void compute_system_out(const State *s,
                        const CalibParams *p,
                        SystemOut *o)
{
    double score;

    memset(o, 0, sizeof(SystemOut));

    /* --- calibrated engine call --- */
    score = run_engine(s, p);

    /* Direct OLS score */
    o->M8         = score;
    o->raw_stress = score;

    /* Informational AI-damping display */
    o->ai_damping = 1.0 / (1.0 + s->ai_capex);

    /* Debt-pressure bar */
    o->debt_pressure =
        clip01(s->debt_gdp / 200.0) * 0.5 +
        clip01(s->int_rev  /  30.0) * 0.5;

    /* Regime classification (matches calibration report) */
    if (score >= 0.85)
        strcpy(o->regime, "Severe");
    else if (score >= 0.65)
        strcpy(o->regime, "Crisis");
    else if (score >= 0.40)
        strcpy(o->regime, "Stress");
    else if (score >= 0.20)
        strcpy(o->regime, "Medium");
    else
        strcpy(o->regime, "Low");

    o->ponr_probability  = clip01(score - 0.7);
    o->stress_dispersion = 0.0;   /* OLS has no dispersion model */
}

/* ---------------------------------------------------------
 * SYSTEM_engine — nonlinear macro-risk engine (research use)
 * Unchanged — not used by MAIN.C or dashboard.
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
    (void)reflexive_mode;
    (void)periods_above;

    memset(o, 0, sizeof(SystemOut));

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

    /* AI damping */
    o->ai_damping = 1.0 - p->ai_coef * o->M9;
    if (o->ai_damping < 0.0) o->ai_damping = 0.0;

    /* Composite stress */
    o->M8 = clip01(o->raw_stress * o->ai_damping);

    if (has_prev) {
        o->M8 = p->terminal_persistence * prev_M8 +
                (1.0 - p->terminal_persistence) * o->M8;
    }

    if (o->M8 > p->hysteresis)
        strcpy(o->regime, "High Risk");
    else
        strcpy(o->regime, "Normal");

    o->ponr_probability  = clip01(o->M8 - 0.7);
    o->stress_dispersion = 0.1;
    o->debt_pressure     = o->M1 + o->M3;
}
