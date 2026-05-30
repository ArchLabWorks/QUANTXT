/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <conio.h>
#include <graph.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "state.h"
#include "scenario.h"
#include "colors.h"
#include "calibpar.h"

/* ---------------------------------------------------------
 * Placeholder model parameters (internal only)
 * --------------------------------------------------------- */
static CalibParams DEF_PARAMS;

static void init_def_params(void)
{
    DEF_PARAMS.m1_w            = 0.0;
    DEF_PARAMS.m3_w            = 0.0;
    DEF_PARAMS.m8_mid          = 0.0;
    DEF_PARAMS.m8_k            = 0.0;
    DEF_PARAMS.ai_coef         = 0.0;
    DEF_PARAMS.hysteresis      = 0.0;
    DEF_PARAMS.regime_penalty  = 0.0;
    DEF_PARAMS.liquidity_floor = 0.0;
    DEF_PARAMS.tail_risk_scale = 0.0;
}

/* ---------------------------------------------------------
 * Built‑in scenarios (params filled at runtime)
 * --------------------------------------------------------- */
Scenario scenarios[MAX_SCENARIOS] = {

    {
        "gfc_2008",
        {
            22, 70, 65, 3, 90,
            0.2, 1.2, 1.0, 0.95, 600, 5, 80,
            0.0, 0.0, 0.4, 0.5,
            0.038, 0.058, -0.030
        }
    },

    {
        "debtceiling_2011",
        {
            18, 95, 60, 6, 120,
            0.3, 1.1, 1.0, 0.95, 500, 8, 90,
            0.0, 0.0, 0.5, 0.6,
            0.030, 0.090, 0.016
        }
    },

    {
        "today_2026",
        {
            22, 125, 55, 7, 120,
            0.4, 1.3, 0.9, 0.95, 480, 12, 85,
            3.5, 3.0, 0.4, 0.6,
            0.030, 0.040, 0.025
        }
    }
};

int scenario_count = 3;

/* ---------------------------------------------------------
 * Copy scenario → state
 * --------------------------------------------------------- */
void fill_state_from_scenario(const Scenario *sc, State *s, double lagged_ai)
{
    *s = sc->state;

    if (lagged_ai != 0.0)
        s->lagged_ai = lagged_ai;
}

/* ---------------------------------------------------------
 * Manual input helper
 * --------------------------------------------------------- */
static double prompt_field(int row, int col, const char *label)
{
    double val = 0.0;

    _settextcolor(COL_YELLOW);
    _settextposition(row, col);
    _outtext((char __far *)label);

    _settextcursor(1);
    while (kbhit()) getch();

    _settextcolor(COL_WHITE);
    scanf("%lf", &val);

    _settextcursor(0);

    return val;
}

/* ---------------------------------------------------------
 * Manual input UI
 * --------------------------------------------------------- */
void manual_input(State *s)
{
    _setvideomode(_TEXTC80);
    _clearscreen(_GCLEARSCREEN);

    _settextcolor(COL_YELLOW);
    _settextposition(1, 1);
    _outtext("================================================");
    _settextposition(2, 1);
    _outtext("         MANUAL INPUT  (IBM PC XT EDITION)");
    _settextposition(3, 1);
    _outtext("================================================");

    _settextcolor(COL_WHITE);
    _settextposition(4, 1);
    _outtext("Enter values and press ENTER after each field.");
    _settextposition(5, 1);
    _outtext("------------------------------------------------");

    /* --- Core fiscal --- */
    s->debt_gdp           = prompt_field( 6, 1, "  Debt/GDP (e.g. 125):          ");
    s->int_rev            = prompt_field( 7, 1, "  Interest/Rev (e.g. 22):       ");
    s->usd_reserve_share  = prompt_field( 8, 1, "  USD Reserve Share (e.g. 55):  ");
    s->cbo_deficit        = prompt_field( 9, 1, "  CBO Deficit (e.g. 7):         ");
    s->xdate              = prompt_field(10, 1, "  X-Date days (e.g. 120):       ");

    /* --- Labor / cycle --- */
    s->sahm               = prompt_field(11, 1, "  Sahm Rule (0.0-1.0):          ");
    s->infl               = prompt_field(12, 1, "  Inflation (e.g. 0.030):       ");
    s->unemp              = prompt_field(13, 1, "  Unemployment (e.g. 0.042):    ");
    s->gdp                = prompt_field(14, 1, "  GDP growth (e.g. 0.025):      ");

    /* --- Financial stress --- */
    s->tail_risk          = prompt_field(15, 1, "  Tail Risk (0.0-1.0):          ");
    s->liq_gap            = prompt_field(16, 1, "  Liquidity Gap (0.0-1.5):      ");
    s->ofr                = prompt_field(17, 1, "  OFR Index (0.0-1.0):          ");
    s->hy_spread          = prompt_field(18, 1, "  HY Spread (e.g. 0.30):        ");
    s->dxy_mom            = prompt_field(19, 1, "  DXY Momentum (0.0-1.0):       ");
    s->oil_price          = prompt_field(20, 1, "  Oil Price (e.g. 80):          ");

    /* --- Technology / reflexivity --- */
    s->ai_capex           = prompt_field(21, 1, "  AI Capex (e.g. 3.5):          ");
    s->lagged_ai          = prompt_field(22, 1, "  Lagged AI (prev period):      ");

    /* --- Geopolitical / sentiment --- */
    s->geopolitical_risk  = prompt_field(23, 1, "  Geo Risk (0.0-1.0):           ");
    s->investor_sentiment = prompt_field(24, 1, "  Sentiment (0.0-1.0):          ");
}

/* ---------------------------------------------------------
 * load_scenario_file_multi — load external .TXT scenarios
 * --------------------------------------------------------- */
int load_scenario_file_multi(const char *filename)
{
    FILE *fp;
    char line[256];
    Scenario temp;
    int count = 0;

    fp = fopen(filename, "r");
    if (!fp)
        return 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        if (sscanf(line,
            "%31s"
            " %lf %lf %lf %lf %lf"
            " %lf %lf %lf %lf %lf"
            " %lf %lf %lf %lf"
            " %lf %lf %lf %lf %lf",
            temp.name,
            &temp.state.int_rev,
            &temp.state.debt_gdp,
            &temp.state.usd_reserve_share,
            &temp.state.cbo_deficit,
            &temp.state.xdate,
            &temp.state.sahm,
            &temp.state.tail_risk,
            &temp.state.liq_gap,
            &temp.state.ofr,
            &temp.state.hy_spread,
            &temp.state.dxy_mom,
            &temp.state.oil_price,
            &temp.state.ai_capex,
            &temp.state.lagged_ai,
            &temp.state.geopolitical_risk,
            &temp.state.investor_sentiment,
            &temp.state.infl,
            &temp.state.unemp,
            &temp.state.gdp) == 20)
        {
            /* Placeholder — real params loaded in main.c */
            memcpy(&temp.params, &DEF_PARAMS, sizeof(CalibParams));

            if (scenario_count < MAX_SCENARIOS) {
                scenarios[scenario_count++] = temp;
                count++;
            }
        }
    }

    fclose(fp);
    return count;
}
