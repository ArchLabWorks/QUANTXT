/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "state.h"     /* Provides Params, SystemOut, State */
#include "calibpar.h"  /* NEW: Provides CalibParams */

/* ---------------------------------------------------------
 * Initialize default nonlinear-engine parameters
 * (Legacy research engine only)
 * --------------------------------------------------------- */
void init_default_params(Params *p);

/* ---------------------------------------------------------
 * compute_system_out — calibrated OLS-engine shim
 *
 * NEW SIGNATURE:
 *     compute_system_out(const State *s,
 *                        const CalibParams *p,
 *                        SystemOut *out)
 *
 * This is the primary scoring path used by MAIN.C and DASH.C.
 * --------------------------------------------------------- */
void compute_system_out(
    const State *s,
    const CalibParams *p,
    SystemOut *out
);

/* ---------------------------------------------------------
 * SYSTEM_engine — nonlinear macro‑risk engine (research)
 *
 * Not used by MAIN.C. Retained for reflexive-mode work.
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

#endif /* SYSTEM_H */
