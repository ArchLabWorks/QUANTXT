/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef STATE_H
#define STATE_H

/* =====================================================
   STATE — live macroeconomic input vector
   ===================================================== */
typedef struct {
    double int_rev;
    double debt_gdp;
    double usd_reserve_share;
    double cbo_deficit;
    double xdate;
    double sahm;
    double tail_risk;
    double liq_gap;
    double ofr;
    double hy_spread;
    double dxy_mom;
    double oil_price;
    double ai_capex;
    double geopolitical_risk;
    double investor_sentiment;
    double lagged_ai;

    /* Required for calibration */
    double infl;
    double unemp;
    double gdp;
} State;

/* =====================================================
   PARAMS — model configuration (moved from system.h)
   ===================================================== */
typedef struct {
    double ai_coef;

    double m1_w;
    double m2_w;
    double m3_w;
    double m4_w;
    double m5_w;
    double m6_w;
    double m7_w;
    double m10_w;
    double m13_w;

    double hysteresis;
    double terminal_persistence;
    double raw_c;
} Params;

/* =====================================================
   SYSTEMOUT — output vector of SYSTEM_engine()
   ===================================================== */
typedef struct {
    double M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M13;

    double raw_stress;
    double ai_damping;

    char   regime[32];

    double ponr_probability;
    double stress_dispersion;
    double debt_pressure;
} SystemOut;

#endif /* STATE_H */
