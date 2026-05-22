/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "state.h"   /* Provides Params, SystemOut, State */

/* ---------------------------------------------------------
 * Initialize default model parameters
 * --------------------------------------------------------- */
void init_default_params(Params *p);
/* ---------------------------------------------------------
 * SYSTEM_engine — main nonlinear macro‑financial engine
 *
 * Arguments:
 *   s              — input State vector
 *   params         — model configuration
 *   prev_M8        — previous composite stress score
 *   has_prev       — whether prev_M8 is valid
 *   reflexive_mode — reserved for future reflexive logic
 *   periods_above  — consecutive periods above hysteresis
 *   out            — output vector (SystemOut)
 *
 * Computes:
 *   - M1–M13 module scores
 *   - raw_stress
 *   - ai_damping
 *   - M8 composite stress
 *   - regime classification
 *   - derived metrics (PONR, dispersion, debt pressure)
 * --------------------------------------------------------- */
void SYSTEM_engine(
    const State *s,
    const Params *params,
    double prev_M8,
    int has_prev,
    int reflexive_mode,
    int periods_above,
    SystemOut *out
);

void compute_system_out(const State *s, SystemOut *o);


#endif /* SYSTEM_H */
