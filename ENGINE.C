/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

/* engine.c -- QUANTXT v1.2 sovereign-risk scoring engine
 *
 * Weights derived via Ridge regression (L2, lambda=2.0)
 * against 46-scenario CALIB2.TXT precedent table, all 19 fields.
 *
 * Ridge is used instead of plain OLS because the feature set
 * contains strong multicollinearity clusters (tail_risk, ofr,
 * liq_gap, inv_sent, hy_spread, sahm are highly intercorrelated).
 * Plain OLS at 46 rows produces sign-flipped weights on 7 of 19
 * features due to the collinearity. Ridge at lambda=2.0 reduces
 * flips to 1 (unemp, small magnitude) with acceptable RMSE cost.
 *
 * Calibration results with these weights (46 scenarios):
 *   SSE  : 0.306674
 *   RMSE : 0.081651
 *   Bias : -0.001126  (near-zero -- no systematic over/under)
 *   Max over-predict  : +0.1603
 *   Max under-predict : -0.2185
 *
 * Calibration history:
 *   Hand-assigned (19 fields)          : RMSE 0.368  (baseline)
 *   OLS 6-field (30-pt dataset)        : RMSE 0.044
 *   OLS 19-field (30-pt dataset)       : RMSE 0.020
 *   OLS 19-field (14-pt CALIB2)        : RMSE 0.493  (sign errors)
 *   Ridge lambda=2.0 (46-pt CALIB2)    : RMSE 0.082  (current)
 *
 * Key findings from this calibration:
 *   - tail_risk (+0.23) remains the dominant stress signal
 *   - hy_spread (+0.17), liq_gap (-0.14), ofr (+0.12),
 *     geo_risk (+0.12), inv_sent (-0.12) are the next tier
 *   - sahm is now correctly positive (+0.079)
 *   - int_rev is now correctly positive (+0.036)
 *   - ai_capex is now correctly positive (+0.007); the prior
 *     negative weight was an artefact of the old training set
 *   - An intercept term (+0.286) is required -- the feature
 *     space does not pass through the origin
 *   - unemp retains a small sign flip (-0.008 vs r=+0.38)
 *     due to residual collinearity with inv_sent; magnitude
 *     is negligible and does not materially affect scores
 *
 * To recalibrate: update CALIB2.TXT and re-run ridge solver
 * (lambda=2.0). Increase lambda if sign flips reappear after
 * adding new scenarios; decrease toward 1.0 if RMSE is too high.
 */

/* engine.c -- QUANTXT v1.2 sovereign-risk scoring engine */

#include <math.h>
#include <stdio.h>

#include "state.h"
#include "calibpar.h"   /* NEW: model parameters */
#include "engine.h"

/* ---------------------------------------------------------
 * run_engine -- parameterized scoring engine
 * --------------------------------------------------------- */
double run_engine(const State *s, const CalibParams *p)
{
    double score = 0.0;

    /* --------------------------------------------------
     * Ridge-calibrated weights (lambda=2.0)
     * Fitted against 46-scenario CALIB2.TXT dataset
     *
     * NOTE:
     *   These weights remain hardcoded for now.
     *   Future versions may replace them with p->fields.
     * -------------------------------------------------- */

    /* Intercept */
    score += 0.2862;

    /* Core macro-financial */
    score += s->int_rev           *  0.0362;
    score += s->debt_gdp          *  0.0096;
    score += s->usd_reserve_share *  0.0004;
    score += s->cbo_deficit       *  0.0059;
    score += s->xdate             * -0.0002;

    /* Labor / cycle */
    score += s->sahm              *  0.0792;
    score += s->infl              *  0.0045;
    score += s->unemp             * -0.0077;
    score += s->gdp               * -0.0101;

    /* Financial stress */
    score += s->tail_risk         *  0.2315;
    score += s->liq_gap           * -0.1377;
    score += s->ofr               *  0.1194;
    score += s->hy_spread         *  0.1674;
    score += s->dxy_mom           *  0.0001;
    score += s->oil_price         *  0.0008;

    /* Technology / reflexivity */
    score += s->ai_capex          *  0.0066;
    score += s->lagged_ai         *  0.0098;

    /* Geopolitical / sentiment */
    score += s->geopolitical_risk *  0.1238;
    score += s->investor_sentiment* -0.1205;

    /* Clamp to [0, 1] */
    if (score < 0.0) score = 0.0;
    if (score > 1.0) score = 1.0;

    return score;
}
