/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

/* qxcalib.c – QuantXT calibration module */

/* qxcalib.c -- QUANTXT calibration module */

#include "qxcalib.h"
#include "state.h"
#include "engine.h"
#include "colors.h"
#include "modload.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <graph.h>
#include <conio.h>

/* ----------------------------------------------------------------
   forward declarations of internal types (unchanged from original)
   ---------------------------------------------------------------- */

typedef struct {
    State  state;
    double target;
    int    has_target;
    char   name[32];        /* scenario name for per-row report */
} QXCalibRow;

/* forward declaration for model writer */
static int qxcalib_write_model(const char *filename,
                               const CalibParams *oldp,
                               const QXCalibResult *r);

/* ----------------------------------------------------------------
   field assignment (float casts removed -- State fields are double)
   ---------------------------------------------------------------- */

static int qxcalib_assign_field(State *st,
                                const char *key,
                                const char *value)
{
    if      (strcmp(key,"int_rev")            ==0) st->int_rev            = atof(value);
    else if (strcmp(key,"debt_gdp")           ==0) st->debt_gdp           = atof(value);
    else if (strcmp(key,"sahm")               ==0) st->sahm               = atof(value);
    else if (strcmp(key,"infl")               ==0) st->infl               = atof(value);
    else if (strcmp(key,"unemp")              ==0) st->unemp              = atof(value);
    else if (strcmp(key,"gdp")                ==0) st->gdp                = atof(value);
    else if (strcmp(key,"tail_risk")          ==0) st->tail_risk          = atof(value);
    else if (strcmp(key,"liq_gap")            ==0) st->liq_gap            = atof(value);
    else if (strcmp(key,"usd_reserve_share")  ==0) st->usd_reserve_share  = atof(value);
    else if (strcmp(key,"cbo_deficit")        ==0) st->cbo_deficit        = atof(value);
    else if (strcmp(key,"xdate")              ==0) st->xdate              = atof(value);
    else if (strcmp(key,"ofr")                ==0) st->ofr                = atof(value);
    else if (strcmp(key,"hy_spread")          ==0) st->hy_spread          = atof(value);
    else if (strcmp(key,"dxy_mom")            ==0) st->dxy_mom            = atof(value);
    else if (strcmp(key,"oil_price")          ==0) st->oil_price          = atof(value);
    else if (strcmp(key,"ai_capex")           ==0) st->ai_capex           = atof(value);
    else if (strcmp(key,"lagged_ai")          ==0) st->lagged_ai          = atof(value);
    else if (strcmp(key,"geopolitical_risk")  ==0) st->geopolitical_risk  = atof(value);
    else if (strcmp(key,"investor_sentiment") ==0) st->investor_sentiment = atof(value);
    else return 1;
    return 0;
}

/* ----------------------------------------------------------------
   parsing helpers (unchanged)
   ---------------------------------------------------------------- */

#define QXCALIB_LINE_LEN 256

static char *qxcalib_trim(char *s)
{
    char *end;
    while (*s==' '||*s=='\t'||*s=='\r'||*s=='\n') s++;
    if (*s=='\0') return s;
    end = s + strlen(s) - 1;
    while (end > s && (*end==' '||*end=='\t'||*end=='\r'||*end=='\n'))
        *end-- = '\0';
    return s;
}

static int qxcalib_parse_kv(char *line, char **out_key, char **out_val)
{
    char *eq = strchr(line, '=');
    if (!eq) return 1;
    *eq = '\0';
    *out_key = qxcalib_trim(line);
    *out_val = qxcalib_trim(eq + 1);
    if (**out_key=='\0' || **out_val=='\0') return 1;
    return 0;
}

/* ----------------------------------------------------------------
   helper: assign row to regime band
   ---------------------------------------------------------------- */

static int get_regime(double target)
{
    if (target < 0.20) return REGIME_LOW;
    if (target < 0.40) return REGIME_MED;
    if (target < 0.65) return REGIME_STRESS;
    if (target < 0.85) return REGIME_CRISIS;
    return REGIME_SEVERE;
}

static const char *regime_label(int r)
{
    switch (r) {
        case REGIME_LOW:    return "Low    (0.00-0.20)";
        case REGIME_MED:    return "Medium (0.20-0.40)";
        case REGIME_STRESS: return "Stress (0.40-0.65)";
        case REGIME_CRISIS: return "Crisis (0.65-0.85)";
        case REGIME_SEVERE: return "Severe (0.85-1.00)";
        default:            return "Unknown";
    }
}

/* ----------------------------------------------------------------
   loader (name field added)
   ---------------------------------------------------------------- */

static void qxcalib_finalize_row(QXCalibRow *rows, int *count,
                                 QXCalibRow *cur)
{
    if (!cur->has_target)           return;
    if (*count >= QXCALIB_MAX_ROWS) return;
    rows[*count] = *cur;
    (*count)++;
}

static int qxcalib_load_precedent(const char *filename,
                                  QXCalibRow *rows,
                                  int max_rows)
{
    FILE *fp;
    char  line[QXCALIB_LINE_LEN];
    char *p, *key, *val;
    QXCalibRow cur;
    int count = 0;
    (void)max_rows;

    fp = fopen(filename, "r");
    if (!fp) return -1;

    memset(&cur, 0, sizeof(cur));

    while (fgets(line, sizeof(line), fp)) {
        p = qxcalib_trim(line);

        if (*p == '\0') {
            qxcalib_finalize_row(rows, &count, &cur);
            memset(&cur, 0, sizeof(cur));
            continue;
        }
        if (*p=='#' || *p==';') continue;

        /* scenario name line: starts with '#name' or is a bare word
           before any '=' -- treat lines without '=' as name lines   */
        if (!strchr(p, '=')) {
            strncpy(cur.name, p, 31);
            cur.name[31] = '\0';
            continue;
        }

        if (qxcalib_parse_kv(p, &key, &val) != 0) continue;

        if (strcmp(key, "target") == 0) {
            cur.target     = atof(val);
            cur.has_target = 1;
        } else if (strcmp(key, "name") == 0) {
            strncpy(cur.name, val, 31);
            cur.name[31] = '\0';
        } else {
            qxcalib_assign_field(&cur.state, key, val);
        }
    }

    qxcalib_finalize_row(rows, &count, &cur);
    fclose(fp);
    return count;
}

/* ----------------------------------------------------------------
   compute -- single pass, accumulates everything
   ---------------------------------------------------------------- */

static void qxcalib_compute(const QXCalibRow *rows, int count,
                            const CalibParams *p,
                            QXCalibResult *out)
{
    double sse        = 0.0;
    double signed_sum = 0.0;
    double max_over   = 0.0;
    double max_under  = 0.0;
    double abs_err;
    double err;
    double model;
    int    band;
    int    i;

    memset(out, 0, sizeof(*out));
    out->max_over  =  0.0;
    out->max_under =  0.0;

    for (i = 0; i < count; i++) {

        /* FIXED: run_engine now receives CalibParams */
        model = run_engine(&rows[i].state, p);

        err   = model - rows[i].target;
        abs_err = err < 0.0 ? -err : err;

        strncpy(out->row_name[i], rows[i].name, 31);
        out->row_name[i][31] = '\0';
        out->row_target[i]   = rows[i].target;
        out->row_model[i]    = model;
        out->row_error[i]    = err;

        sse        += err * err;
        signed_sum += err;

        if (err > max_over) {
            max_over = err;
            strncpy(out->max_over_name, rows[i].name, 31);
            out->max_over_name[31] = '\0';
        }
        if (err < max_under) {
            max_under = err;
            strncpy(out->max_under_name, rows[i].name, 31);
            out->max_under_name[31] = '\0';
        }

        band = get_regime(rows[i].target);
        out->regime_count[band]++;
        out->regime_sse[band] += err * err;
        if (abs_err > out->regime_max_err[band])
            out->regime_max_err[band] = abs_err;
    }

    out->rows              = count;
    out->total_error       = sse;
    out->mean_error        = (count > 0) ? sse / (double)count : 0.0;
    out->mean_signed_error = (count > 0) ? signed_sum / (double)count : 0.0;
    out->max_over          = max_over;
    out->max_under         = max_under;
}


/* ----------------------------------------------------------------
   result writer -- full Phase 1 report
   ---------------------------------------------------------------- */

#define OVER_THRESHOLD 0.020   /* flag rows worse than this */

static void write_separator(FILE *fp, char c, int width)
{
    int i;
    for (i = 0; i < width; i++) fputc(c, fp);
    fputc('\n', fp);
}

static int qxcalib_write_result(const char *filename,
                                const QXCalibResult *r)
{
    FILE  *fp;
    int    i, band;
    double regime_mse;
    double abs_err;
    const char *bias_label;

    fp = fopen(filename, "w");
    if (!fp) return -1;

    /* ---- header ---- */
    write_separator(fp, '=', 60);
    fprintf(fp, "QUANTXT CALIBRATION REPORT\n");
    write_separator(fp, '=', 60);
    fprintf(fp, "\n");

    /* ---- aggregate metrics ---- */
    fprintf(fp, "AGGREGATE METRICS\n");
    write_separator(fp, '-', 40);
    fprintf(fp, "Rows tested    : %d\n",     r->rows);
    fprintf(fp, "Total SSE      : %.6f\n",   r->total_error);
    fprintf(fp, "Mean error     : %.6f\n",   r->mean_error);
    fprintf(fp, "RMSE           : %.6f\n",
            r->mean_error > 0.0 ? sqrt(r->mean_error) : 0.0);
    fprintf(fp, "\n");

    /* ---- bias analysis ---- */
    fprintf(fp, "BIAS ANALYSIS\n");
    write_separator(fp, '-', 40);
    fprintf(fp, "Mean signed error : %+.6f\n", r->mean_signed_error);

    if (r->mean_signed_error >  0.005)       bias_label = "OVER-PREDICTING";
    else if (r->mean_signed_error < -0.005)  bias_label = "UNDER-PREDICTING";
    else                                     bias_label = "LOW (within +/-0.005)";
    fprintf(fp, "Systematic bias   : %s\n",    bias_label);
    fprintf(fp, "Max over-predict  : %+.6f  (%s)\n",
            r->max_over,  r->max_over_name[0]  ? r->max_over_name  : "n/a");
    fprintf(fp, "Max under-predict : %+.6f  (%s)\n",
            r->max_under, r->max_under_name[0] ? r->max_under_name : "n/a");
    fprintf(fp, "\n");

    /* ---- regime breakdown ---- */
    fprintf(fp, "REGIME BREAKDOWN\n");
    write_separator(fp, '-', 60);
    fprintf(fp, "%-20s  %5s  %10s  %10s\n",
            "Regime", "Rows", "MSE", "Max |err|");
    write_separator(fp, '-', 60);

    for (band = 0; band < REGIME_COUNT; band++) {
        if (r->regime_count[band] == 0) {
            fprintf(fp, "%-20s  %5d  %10s  %10s\n",
                    regime_label(band), 0, "---", "---");
        } else {
            regime_mse = r->regime_sse[band] / (double)r->regime_count[band];
            fprintf(fp, "%-20s  %5d  %10.6f  %10.6f\n",
                    regime_label(band),
                    r->regime_count[band],
                    regime_mse,
                    r->regime_max_err[band]);
        }
    }
    fprintf(fp, "\n");

    /* ---- per-row detail table ---- */
    fprintf(fp, "PER-SCENARIO DETAIL\n");
    write_separator(fp, '-', 60);
    fprintf(fp, "%-28s  %7s  %7s  %8s\n",
            "Scenario", "Target", "Model", "Error");
    write_separator(fp, '-', 60);

    for (i = 0; i < r->rows; i++) {
        abs_err = r->row_error[i] < 0.0
                  ? -r->row_error[i]
                  :  r->row_error[i];

        fprintf(fp, "%-28s  %7.4f  %7.4f  %+8.4f%s\n",
                r->row_name[i][0] ? r->row_name[i] : "(unnamed)",
                r->row_target[i],
                r->row_model[i],
                r->row_error[i],
                abs_err > OVER_THRESHOLD ? "  <<" : "");
    }
    fprintf(fp, "\n");
    fprintf(fp, "  << flagged: |error| > %.3f\n", OVER_THRESHOLD);
    fprintf(fp, "\n");

    write_separator(fp, '=', 60);
    fprintf(fp, "END OF REPORT\n");
    write_separator(fp, '=', 60);

    fclose(fp);
    return 0;
}

/* ----------------------------------------------------------------
   arbiter writer -- compact machine-readable summary
   ---------------------------------------------------------------- */

#define QXCALIB_ARBITER_FILE "CALIB_AR.TXT"

static int qxcalib_write_arbiter(const char *filename,
                                 const QXCalibResult *r)
{
    FILE  *fp;
    int    band;
    double regime_mse;

    fp = fopen(filename, "w");
    if (!fp) return -1;

    /* simple key=value format, DOS- and C89-friendly */

    fprintf(fp, "rows=%d\n", r->rows);
    fprintf(fp, "total_sse=%.10f\n", r->total_error);
    fprintf(fp, "mean_error=%.10f\n", r->mean_error);
    fprintf(fp, "rmse=%.10f\n",
            r->mean_error > 0.0 ? sqrt(r->mean_error) : 0.0);
    fprintf(fp, "mean_signed_error=%.10f\n", r->mean_signed_error);
    fprintf(fp, "max_over=%.10f\n", r->max_over);
    fprintf(fp, "max_under=%.10f\n", r->max_under);

    for (band = 0; band < REGIME_COUNT; band++) {
        if (r->regime_count[band] == 0) {
            fprintf(fp, "regime_%d_rows=0\n", band);
            fprintf(fp, "regime_%d_mse=0.0\n", band);
            fprintf(fp, "regime_%d_maxerr=0.0\n", band);
        } else {
            regime_mse = r->regime_sse[band]
                         / (double)r->regime_count[band];
            fprintf(fp, "regime_%d_rows=%d\n",
                    band, r->regime_count[band]);
            fprintf(fp, "regime_%d_mse=%.10f\n",
                    band, regime_mse);
            fprintf(fp, "regime_%d_maxerr=%.10f\n",
                    band, r->regime_max_err[band]);
        }
    }

    fclose(fp);
    return 0;
}

/* ----------------------------------------------------------------
   public entry point (unchanged signature)
   ---------------------------------------------------------------- */

int qxcalib_run(const char *precedent_filename,
                const char *result_filename,
                QXCalibResult *out_result)
{
    static QXCalibRow rows[QXCALIB_MAX_ROWS];
    QXCalibResult res;
    CalibParams model_params;
    int n;
    int rc;

    /* Load calibrated model parameters (same as main.c) */
    load_model_params("MODL.TXT", &model_params);

    n = qxcalib_load_precedent(precedent_filename, rows, QXCALIB_MAX_ROWS);
    if (n < 0) return 1;

    /* FIXED: pass model_params into compute */
    qxcalib_compute(rows, n, &model_params, &res);
    /* write updated model parameters */
    qxcalib_compute(rows, n, &model_params, &res);

    /* write human-readable report */
    if (qxcalib_write_result(result_filename, &res) != 0)
        return 2;

    /* write compact arbiter file */
    rc = qxcalib_write_arbiter(QXCALIB_ARBITER_FILE, &res);

    /* NEW: write updated model parameters */
    qxcalib_write_model("MODL.TXT", &model_params, &res);

    if (out_result)
        *out_result = res;


    return 0;
}

static int qxcalib_write_model(const char *filename,
                               const CalibParams *oldp,
                               const QXCalibResult *r)
{
    FILE *fp;
    CalibParams newp;
    double adj;

    /* adjustment factor based on signed bias */
    adj = 1.0 - r->mean_signed_error;

    /* apply adjustment */
    newp.m1_w            = oldp->m1_w            * adj;
    newp.m3_w            = oldp->m3_w            * adj;
    newp.m8_mid          = oldp->m8_mid          * adj;
    newp.m8_k            = oldp->m8_k            * adj;
    newp.ai_coef         = oldp->ai_coef         * adj;
    newp.hysteresis      = oldp->hysteresis      * adj;
    newp.regime_penalty  = oldp->regime_penalty  * adj;
    newp.liquidity_floor = oldp->liquidity_floor * adj;
    newp.tail_risk_scale = oldp->tail_risk_scale * adj;

    fp = fopen(filename, "w");
    if (!fp) return -1;

    fprintf(fp, "m1_w=%.10f\n",            newp.m1_w);
    fprintf(fp, "m3_w=%.10f\n",            newp.m3_w);
    fprintf(fp, "m8_mid=%.10f\n",          newp.m8_mid);
    fprintf(fp, "m8_k=%.10f\n",            newp.m8_k);
    fprintf(fp, "ai_coef=%.10f\n",         newp.ai_coef);
    fprintf(fp, "hysteresis=%.10f\n",      newp.hysteresis);
    fprintf(fp, "regime_penalty=%.10f\n",  newp.regime_penalty);
    fprintf(fp, "liquidity_floor=%.10f\n", newp.liquidity_floor);
    fprintf(fp, "tail_risk_scale=%.10f\n", newp.tail_risk_scale);

    fclose(fp);
    return 0;
}

/* ----------------------------------------------------------------
   screen display with spacebar paging
   ---------------------------------------------------------------- */

#define DISPLAY_LINES  22   /* rows per page (leaves room for prompt) */
#define MAX_DISP_LINES 120  /* enough for 60 scenarios + headers      */
#define DISP_LINE_LEN   64  /* max chars per display line             */

#define PUSHLINE(s) \
    if (total < MAX_DISP_LINES) { \
        strncpy(lines[total], (s), DISP_LINE_LEN-1); \
        lines[total][DISP_LINE_LEN-1] = '\0'; \
        total++; \
    }

#define PUSHFMT(...) \
    sprintf(buf, __VA_ARGS__); \
    PUSHLINE(buf)


/* build the display buffer then page through it */
void qxcalib_display(const QXCalibResult *r)
{
    /* line buffer -- static so it doesn't hit the stack */
    static char lines[MAX_DISP_LINES][DISP_LINE_LEN];
    int   total = 0;
    int   top   = 0;
    int   i, band, page_end;
    int   ch;
    double abs_err;
    double regime_mse;
    const char *bias_label;
    char buf[DISP_LINE_LEN];

    /* ---- build line buffer ---- */

    PUSHLINE("============================================");
    PUSHLINE("QUANTXT CALIBRATION REPORT");
    PUSHLINE("============================================");
    PUSHLINE("");

    PUSHLINE("AGGREGATE METRICS");
    PUSHLINE("--------------------------------------------");
    PUSHFMT ("Rows tested    : %d",           r->rows);
    PUSHFMT ("Total SSE      : %.6f",          r->total_error);
    PUSHFMT ("Mean error     : %.6f",          r->mean_error);
    PUSHFMT ("RMSE           : %.6f",          sqrt(r->mean_error));
    PUSHLINE("");

    PUSHLINE("BIAS ANALYSIS");
    PUSHLINE("--------------------------------------------");
    PUSHFMT ("Mean signed err: %+.6f",         r->mean_signed_error);

    if      (r->mean_signed_error >  0.005) bias_label = "OVER-PREDICTING";
    else if (r->mean_signed_error < -0.005) bias_label = "UNDER-PREDICTING";
    else                                    bias_label = "LOW (+/-0.005)";
    PUSHFMT ("Systematic bias: %s",            bias_label);
    PUSHFMT ("Max over       : %+.6f  (%s)",
             r->max_over,
             r->max_over_name[0]  ? r->max_over_name  : "n/a");
    PUSHFMT ("Max under      : %+.6f  (%s)",
             r->max_under,
             r->max_under_name[0] ? r->max_under_name : "n/a");
    PUSHLINE("");

    PUSHLINE("REGIME BREAKDOWN");
    PUSHLINE("--------------------------------------------");
    PUSHLINE("Regime               Rows    MSE     MaxErr");
    PUSHLINE("--------------------------------------------");

    for (band = 0; band < REGIME_COUNT; band++) {
        if (r->regime_count[band] == 0) {
            PUSHFMT("%-20s  %4d  ---     ---",
                    regime_label(band), 0);
        } else {
            regime_mse = r->regime_sse[band]
                         / (double)r->regime_count[band];
            PUSHFMT("%-20s  %4d  %.4f  %.4f",
                    regime_label(band),
                    r->regime_count[band],
                    regime_mse,
                    r->regime_max_err[band]);
        }
    }
    PUSHLINE("");

    PUSHLINE("PER-SCENARIO DETAIL");
    PUSHLINE("--------------------------------------------");
    PUSHLINE("Scenario             Target  Model   Error");
    PUSHLINE("--------------------------------------------");

    for (i = 0; i < r->rows; i++) {
        abs_err = r->row_error[i] < 0.0
                  ? -r->row_error[i]
                  :  r->row_error[i];
        PUSHFMT("%-20s %6.4f  %6.4f  %+6.4f%s",
                r->row_name[i][0] ? r->row_name[i] : "(unnamed)",
                r->row_target[i],
                r->row_model[i],
                r->row_error[i],
                abs_err > OVER_THRESHOLD ? " <<" : "");
    }

    PUSHLINE("");
    PUSHFMT ("<< flagged: |error| > %.3f", OVER_THRESHOLD);
    PUSHLINE("============================================");

    /* ---- page display loop ---- */
    for (;;) {
        _clearscreen(_GCLEARSCREEN);

        page_end = top + DISPLAY_LINES;
        if (page_end > total) page_end = total;

        for (i = top; i < page_end; i++) {
            _settextposition(i - top + 1, 1);
            _settextcolor(COL_NORMAL);
            _outtext(lines[i]);
        }

        /* prompt line at row 24 */
        _settextposition(24, 1);
        _settextcolor(COL_FOOTER);

        if (page_end >= total) {
            /* last page */
            _outtext("END OF REPORT -- press any key to return");
            getch();
            return;
        } else {
            /* more pages remain */
            sprintf(buf, "--- page %d/%d --- SPACE=next  ESC=exit",
                    top / DISPLAY_LINES + 1,
                    (total + DISPLAY_LINES - 1) / DISPLAY_LINES);
            _outtext(buf);

            ch = getch();
            if (ch == 27) return;
            if (ch == ' ') {
                top += DISPLAY_LINES;
            }
        }
    }
}
