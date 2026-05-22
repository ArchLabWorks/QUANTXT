/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef MODULES_H
#define MODULES_H

#include "state.h"

/*
 * modules.h — Normalized macro‑risk module interface (M1–M13)
 *
 * Each module returns a 0–1 normalized stress score using clip01().
 * These functions are pure, deterministic, and side‑effect free.
 *
 * They are intentionally simple: SYSTEM_engine() handles weighting,
 * damping, smoothing, and regime classification.
 */

/* Interest burden stress (depends on M3) */
double M1(double int_rev, double M3_score);

/* Liquidity + tail‑risk stress (depends on M5) */
double M2(double tail_risk, double liq_gap, double M5_score);

/* X‑date + fiscal stress */
double M3(double xdate, double cbo_deficit);

/* USD reserve share decline */
double M4(double usd_reserve_share);

/* Credit stress (OFR + HY spreads) */
double M5(double ofr, double hy_spread);

/* Cycle stress (Sahm + AI reflexivity) */
double M6(double sahm, double M9_score, double lagged_ai);

/* Dollar + oil shock stress */
double M7(double dxy_mom, double oil_price);

/* AI capex stabilizer */
double M9(double ai_capex);

/* Geopolitical risk */
double M10(double geopolitical_risk);

/* Behavioral sentiment inversion */
double M13(double investor_sentiment);

#endif
