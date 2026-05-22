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



/* ---------------------------------------------------------
 * Built‑in scenarios
 * --------------------------------------------------------- */
Scenario scenarios[256] = {
    {"gfc_2008",
     22, 70,  65, 3, 90,  0.2, 1.2,
     1.0, 0.95, 600, 5, 80, 1.0, 0.5, 0.4},

    {"debtceiling_2011",
     18, 95,  60, 6, 120, 0.3, 1.1,
     1.0, 0.95, 500, 8, 90, 1.2, 0.6, 0.5},

    {"today_2026",
     22, 125, 55, 7, 120, 0.4, 1.3,
     0.9, 0.95, 480, 12, 85, 3.5, 0.6, 0.4}
};

int scenario_count = 3;

/* ---------------------------------------------------------
 * Copy scenario → state
 * --------------------------------------------------------- */
void fill_state_from_scenario(
    const Scenario *sc, State *s, double lagged_ai)
{
    s->int_rev            = sc->int_rev;
    s->debt_gdp           = sc->debt_gdp;
    s->usd_reserve_share  = sc->usd_reserve_share;
    s->cbo_deficit        = sc->cbo_deficit;
    s->xdate              = sc->xdate;
    s->sahm               = sc->sahm;
    s->tail_risk          = sc->tail_risk;
    s->liq_gap            = sc->liq_gap;
    s->ofr                = sc->ofr;
    s->hy_spread          = sc->hy_spread;
    s->dxy_mom            = sc->dxy_mom;
    s->oil_price          = sc->oil_price;
    s->ai_capex           = sc->ai_capex;
    s->geopolitical_risk  = sc->geopolitical_risk;
    s->investor_sentiment = sc->investor_sentiment;

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

    _settextcursor(1);            /* show cursor */
    while (kbhit()) getch();      /* flush leftover keys */

    _settextcolor(COL_WHITE);
    scanf("%lf", &val);

    _settextcursor(0);            /* hide cursor */

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

    s->debt_gdp           = prompt_field( 6, 1, "  Debt/GDP (e.g. 125):          ");
    s->int_rev            = prompt_field( 7, 1, "  Interest/Rev (e.g. 22):       ");
    s->usd_reserve_share  = prompt_field( 8, 1, "  USD Reserve Share (e.g. 55):  ");
    s->cbo_deficit        = prompt_field( 9, 1, "  CBO Deficit (e.g. 7):         ");
    s->xdate              = prompt_field(10, 1, "  X-Date days (e.g. 120):       ");
    s->sahm               = prompt_field(11, 1, "  Sahm Rule (0.0-1.0):          ");
    s->ai_capex           = prompt_field(12, 1, "  AI Capex (e.g. 3.5):          ");
    s->geopolitical_risk  = prompt_field(13, 1, "  Geo Risk (0.0-1.0):           ");

    /* return to graphics/text mode as needed by caller */
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
            "%31s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            temp.name,
            &temp.int_rev,
            &temp.debt_gdp,
            &temp.usd_reserve_share,
            &temp.cbo_deficit,
            &temp.xdate,
            &temp.sahm,
            &temp.tail_risk,
            &temp.liq_gap,
            &temp.ofr,
            &temp.hy_spread,
            &temp.dxy_mom,
            &temp.oil_price,
            &temp.ai_capex,
            &temp.geopolitical_risk,
            &temp.investor_sentiment) == 16)
        {
            scenarios[scenario_count++] = temp;
            count++;
        }

    }

    fclose(fp);
    return count;
}
