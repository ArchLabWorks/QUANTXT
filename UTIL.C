/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <math.h>
#include "util.h"

/* ---------------------------------------------------------
 * clip01 — clamp x to [0,1]
 * --------------------------------------------------------- */
double clip01(double x)
{
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

/* ---------------------------------------------------------
 * LOGF — logistic sigmoid centered at 'mid'
 * --------------------------------------------------------- */
double LOGF(double x, double mid, double k)
{
    return 1.0 / (1.0 + exp(-k * (x - mid)));
}

/* ---------------------------------------------------------
 * SMOOTH_STEP — Hermite-smoothed multi-threshold step
 *
 * th[] must be ascending. n >= 2.
 * k = smoothing width fraction (0 = hard step).
 * --------------------------------------------------------- */
double SMOOTH_STEP(double x, const double *th, int n, double k)
{
    int i;
    double band, lo, hi, t;

    if (n < 2) return 0.0;               /* defensive */
    if (x < th[0]) return 0.0;
    if (x >= th[n - 1]) return 1.0;

    for (i = 0; i < n - 1; i++) {
        if (x < th[i + 1]) {

            band = (double)i / (double)(n - 1);

            hi = th[i + 1];
            lo = hi - k * (th[i + 1] - th[i]);

            if (x < lo)
                return band;

            t = (x - lo) / (hi - lo);
            t = t * t * (3.0 - 2.0 * t); /* Hermite */

            return band + t * (1.0 / (double)(n - 1));
        }
    }

    return 1.0;
}

/* ---------------------------------------------------------
 * PWL — continuous piecewise linear function
 *
 * bp[] must be ascending. n >= 2.
 * --------------------------------------------------------- */
double PWL(double x, const double *bp, const double *slopes, int n)
{
    int i;
    double y = 0.0;

    if (n < 2) return 0.0;               /* defensive */

    if (x <= bp[0])
        return 0.0;

    for (i = 0; i < n - 1; i++) {
        if (x < bp[i + 1])
            return y + slopes[i] * (x - bp[i]);

        y += slopes[i] * (bp[i + 1] - bp[i]);
    }

    return y + slopes[n - 1] * (x - bp[n - 1]);
}

/* ---------------------------------------------------------
 * SPL — linear spline with optional irreversible ratchet
 *
 * knots[] must be ascending. n >= 2.
 *
 * IMPORTANT FIX:
 *   The original used a static global spl_max, causing
 *   cross-scenario contamination. Now the caller provides
 *   a pointer to its own ratchet state.
 *
 * irreversible_state:
 *   - If NULL → no ratchet
 *   - If non-NULL → ratchet stored per caller
 * --------------------------------------------------------- */
double SPL(
    double x,
    const double *knots,
    const double *vals,
    int n,
    int irreversible,
    double *irreversible_state
){
    int i;
    double t, result;

    if (n < 2) return 0.0;               /* defensive */

    if (x <= knots[0])
        result = vals[0];
    else if (x >= knots[n - 1])
        result = vals[n - 1];
    else {
        result = vals[n - 1];
        for (i = 0; i < n - 1; i++) {
            if (x < knots[i + 1]) {
                t = (x - knots[i]) / (knots[i + 1] - knots[i]);
                result = vals[i] + t * (vals[i + 1] - vals[i]);
                break;
            }
        }
    }

    /* Per-call irreversible ratchet */
    if (irreversible && irreversible_state) {
        if (result > *irreversible_state)
            *irreversible_state = result;
        return *irreversible_state;
    }

    return result;
}
