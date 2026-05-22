/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef UTIL_H
#define UTIL_H

double clip01(double x);
double LOGF(double x, double mid, double k);
double SMOOTH_STEP(double x, const double *th, int n, double k);
double PWL(double x, const double *bp, const double *slopes, int n);

/*
 * SPL — linear spline with optional irreversible ratchet
 *
 * irreversible_state:
 *   - NULL → no ratchet
 *   - non‑NULL → caller‑maintained ratchet memory
 */
double SPL(
    double x,
    const double *knots,
    const double *vals,
    int n,
    int irreversible,
    double *irreversible_state
);

#endif
