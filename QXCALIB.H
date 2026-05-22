/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

/* qxcalib.h – QuantXT calibration module */

#ifndef QXCALIB_H
#define QXCALIB_H

#include "state.h"

#define QXCALIB_MAX_ROWS 64

/* Regime band indices */
#define REGIME_LOW    0   /* 0.00 - 0.20 */
#define REGIME_MED    1   /* 0.20 - 0.40 */
#define REGIME_STRESS 2   /* 0.40 - 0.65 */
#define REGIME_CRISIS 3   /* 0.65 - 0.85 */
#define REGIME_SEVERE 4   /* 0.85 - 1.00 */
#define REGIME_COUNT  5

typedef struct {
    /* --- aggregate --- */
    int    rows;
    double total_error;
    double mean_error;

    /* --- bias --- */
    double mean_signed_error;
    double max_over;            /* largest positive error */
    double max_under;           /* largest negative error (most negative) */
    char   max_over_name[32];
    char   max_under_name[32];

    /* --- regime breakdown --- */
    int    regime_count[REGIME_COUNT];
    double regime_sse[REGIME_COUNT];
    double regime_max_err[REGIME_COUNT];  /* worst abs error per band */

    /* --- per-row (for detail table in result file) --- */
    char   row_name[QXCALIB_MAX_ROWS][32];
    double row_target[QXCALIB_MAX_ROWS];
    double row_model[QXCALIB_MAX_ROWS];
    double row_error[QXCALIB_MAX_ROWS];   /* signed: model - target */
} QXCalibResult;

int qxcalib_run(const char *precedent_filename,
                const char *result_filename,
                QXCalibResult *out_result);

/* Display calibration result to screen with spacebar paging */
void qxcalib_display(const QXCalibResult *r);

#endif /* QXCALIB_H */
