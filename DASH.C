/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

/* dash.c -- QUANTXT fiscal dominance dashboard */

#include <graph.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>   /* for MK_FP() */

#include "state.h"
#include "colors.h"
#include "dash.h"

/* ---------------------------------------------------------
 * Layout constants
 * --------------------------------------------------------- */
#define DASH_WIDTH     60
#define DASH_OFFSET    ((80 - DASH_WIDTH) / 2)

#define LABEL_COL      (DASH_OFFSET + 2)
#define BAR_COL        (DASH_OFFSET + 26)   /* corrected alignment */
#define VALUE_COL      (BAR_COL + 22)       /* after [####################] */

#define DASH_FOOTER    25

/* ---------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------- */
static void out_double(double v)
{
    char buf[16];
    sprintf(buf, "%g", v);
    _outtext(buf);
}

static void draw_divider(int row)
{
    _settextcolor(COL_FOOTER);
    _settextposition(row, DASH_OFFSET);
    _outtext("------------------------------------------------------------");
    _settextcolor(COL_NORMAL);
}

static void draw_label(int row, const char *label)
{
    _settextposition(row, LABEL_COL);
    _outtext((char __far *)label);
}

/* ---------------------------------------------------------
 * Risk bar renderer
 * --------------------------------------------------------- */

/* ---------------------------------------------------------
 * Centered risk bar renderer
 * --------------------------------------------------------- */
static void draw_risk_bar(double value, int row)
{
    int filled = (int)(value * 20.0);
    int i;

    /* Bar geometry: 24 characters wide, centered in 80 columns */
    #define DASH_BAR_WIDTH  24
    #define DASH_BAR_OFFSET ((80 - DASH_BAR_WIDTH) / 2)

    if (filled < 0)  filled = 0;
    if (filled > 20) filled = 20;

    /* Draw opening bracket */
    _settextposition(row, DASH_BAR_OFFSET);
    _settextcolor(COL_NORMAL);
    _outtext("[");

    /* Draw filled portion */
    _settextcolor(value > 0.7 ? COL_HIGH :
                 (value > 0.4 ? COL_MED : COL_LOW));

    for (i = 0; i < filled; ++i)
        _outtext("#");

    /* Draw empty portion */
    _settextcolor(COL_NORMAL);
    for (i = filled; i < 20; ++i)
        _outtext(" ");

    /* Draw closing bracket */
    _outtext("]");

    /* Print numeric value aligned to the right of the bar */
    _settextposition(row, DASH_BAR_OFFSET + DASH_BAR_WIDTH + 2);
    out_double(value);
}

/* ---------------------------------------------------------
 * draw_dashboard -- centered XT-style dashboard
 * --------------------------------------------------------- */
void draw_dashboard(const SystemOut *o, const State *s)
{
    _clearscreen(_GCLEARSCREEN);

    /* ---- HEADER ---- */
    _settextcolor(COL_HEADER);
    _settextposition(1, DASH_OFFSET + 17);
    _outtext("QUANTXT v1.14");
    _settextposition(2, DASH_OFFSET + 12);
    _outtext("FISCAL DOMINANCE DASHBOARD");
    _settextcolor(COL_NORMAL);

    draw_divider(3);

    /* ---- INPUTS ---- */
    _settextcolor(COL_HEADER);
    _settextposition(4, DASH_OFFSET);
    _outtext("INPUTS");
    _settextcolor(COL_NORMAL);

    draw_label(5,  "Debt/GDP          : "); out_double(s->debt_gdp);
    draw_label(6,  "Interest/Revenue  : "); out_double(s->int_rev);
    draw_label(7,  "USD Reserve Share : "); out_double(s->usd_reserve_share);
    draw_label(8,  "CBO Deficit       : "); out_double(s->cbo_deficit);
    draw_label(9,  "X-Date (days)     : "); out_double(s->xdate);
    draw_label(10, "Sahm Rule         : "); out_double(s->sahm);

    draw_divider(11);

    /* ---- RISK VIEWS ---- */
    _settextcolor(COL_HEADER);
    _settextposition(12, DASH_OFFSET);
    _outtext("RISK VIEWS");
    _settextcolor(COL_NORMAL);

    draw_label(13, "Tail Risk        "); draw_risk_bar(s->tail_risk,          13);
    draw_label(14, "Liquidity Gap    "); draw_risk_bar(s->liq_gap,            14);
    draw_label(15, "Debt Pressure    "); draw_risk_bar(o->debt_pressure,      15);
    draw_label(16, "OFR Index        "); draw_risk_bar(s->ofr,                16);
    draw_label(17, "HY Spread        "); draw_risk_bar(s->hy_spread,          17);
    draw_label(18, "DXY Momentum     "); draw_risk_bar(s->dxy_mom,            18);
    draw_label(19, "Oil Price        "); draw_risk_bar(s->oil_price,          19);
    draw_label(20, "AI Capex         "); draw_risk_bar(s->ai_capex,           20);
    draw_label(21, "Geo Risk         "); draw_risk_bar(s->geopolitical_risk,  21);
    draw_label(22, "Sentiment        "); draw_risk_bar(s->investor_sentiment, 22);

    draw_divider(23);

    /* ---- SYSTEM OUTPUT ---- */
    _settextcolor(COL_HEADER);
    _settextposition(24, DASH_OFFSET);
    _outtext("SYSTEM OUTPUT");
    _settextcolor(COL_NORMAL);

    _settextposition(DASH_FOOTER, DASH_OFFSET);
    _settextcolor(COL_FOOTER);
    _outtext("SPACE for output  ESC to return");
    _settextcolor(COL_NORMAL);

    for (;;) {
        int ch = getch();

        if (ch == 27)
            return;

        if (ch == 32) {
            /* ---- OUTPUT PAGE ---- */
            _clearscreen(_GCLEARSCREEN);

            _settextcolor(COL_HEADER);
            _settextposition(1, DASH_OFFSET + 12);
            _outtext("SYSTEM OUTPUT");
            _settextcolor(COL_NORMAL);

            draw_divider(2);

            draw_label(4,  "Raw Stress        : "); out_double(o->raw_stress);
            draw_label(5,  "AI Damping        : "); out_double(o->ai_damping);
            draw_label(6,  "Composite Stress  : "); out_double(o->M8);
            draw_label(8,  "Regime            : ");
            _outtext((char __far *)o->regime);
            draw_label(10, "PONR Probability  : "); out_double(o->ponr_probability);

            draw_divider(12);

            _settextposition(DASH_FOOTER, DASH_OFFSET);
            _settextcolor(COL_FOOTER);
            _outtext("ESC to return to dashboard");
            _settextcolor(COL_NORMAL);

            while (getch() != 27) {
                /* wait for ESC */
            }

            draw_dashboard(o, s);
            return;
        }
    }
}
