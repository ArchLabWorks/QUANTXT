/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <string.h>
#include "state.h"
#include "modules.h"

/* simple clamp */
static double clip01(double x)
{
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

/* Interest burden stress (depends on M3) */
double M1(double int_rev, double M3_score)
{
    /* int_rev ~ 0–30% → map to 0–1, then blend with M3 */
    double base = clip01(int_rev / 30.0);
    return clip01(0.6 * base + 0.4 * M3_score);
}

/* Liquidity + tail‑risk stress (depends on M5) */
double M2(double tail_risk, double liq_gap, double M5_score)
{
    double lr  = clip01(tail_risk);
    double lg  = clip01(liq_gap);
    double mix = 0.5 * lr + 0.5 * lg;
    return clip01(0.5 * mix + 0.5 * M5_score);
}

/* X‑date + fiscal stress */
double M3(double xdate, double cbo_deficit)
{
    /* shorter xdate → higher stress; larger deficit → higher stress */
    double x  = clip01((180.0 - xdate) / 180.0);
    double cd = clip01(cbo_deficit / 10.0);
    return clip01(0.6 * x + 0.4 * cd);
}

/* USD reserve share decline */
double M4(double usd_reserve_share)
{
    /* lower share → higher stress; assume 80% → calm, 40% → high stress */
    double norm = clip01((80.0 - usd_reserve_share) / 40.0);
    return norm;
}

/* Credit stress (OFR + HY spreads) */
double M5(double ofr, double hy_spread)
{
    double o  = clip01(ofr);
    double hy = clip01(hy_spread);
    return clip01(0.5 * o + 0.5 * hy);
}

/* Cycle stress (Sahm + AI reflexivity) */
double M6(double sahm, double M9_score, double lagged_ai)
{
    double cyc = clip01(sahm / 2.0);          /* sahm > 0.5 → stress */
    double ref = clip01(lagged_ai / 5.0);     /* high past AI → some cushion */
    double damp = M9_score;                   /* stabilizer */
    return clip01(0.7 * cyc + 0.3 * ref) * (1.0 - 0.3 * damp);
}

/* Dollar + oil shock stress */
double M7(double dxy_mom, double oil_price)
{
    double d = clip01(dxy_mom);
    double o = clip01(oil_price / 150.0);     /* 150 as rough high */
    return clip01(0.5 * d + 0.5 * o);
}

/* AI capex stabilizer */
double M9(double ai_capex)
{
    /* more AI capex → more stabilizer, saturating */
    return clip01(ai_capex / 5.0);
}

/* Geopolitical risk */
double M10(double geopolitical_risk)
{
    return clip01(geopolitical_risk);
}

/* Behavioral sentiment inversion */
double M13(double investor_sentiment)
{
    /* euphoric sentiment → higher tail risk (contrarian) */
    double s = clip01(investor_sentiment);
    return s;
}

/* ---------------------------------------------------------
 * System-level aggregator
 * --------------------------------------------------------- */
void compute_system_out(const State *s, SystemOut *o)
{
    double m3  = M3(s->xdate, s->cbo_deficit);
    double m1  = M1(s->int_rev, m3);
    double m5  = M5(s->ofr, s->hy_spread);
    double m2  = M2(s->tail_risk, s->liq_gap, m5);
    double m9  = M9(s->ai_capex);
    double m6  = M6(s->sahm, m9, s->lagged_ai);
    double m4  = M4(s->usd_reserve_share);
    double m7  = M7(s->dxy_mom, s->oil_price);
    double m10 = M10(s->geopolitical_risk);
    double m13 = M13(s->investor_sentiment);

    /* simple composite */
    double raw =
        0.15 * m1  +
        0.15 * m2  +
        0.10 * m3  +
        0.10 * m4  +
        0.10 * m5  +
        0.10 * m6  +
        0.10 * m7  +
        0.10 * m10 +
        0.10 * m13;

    o->raw_stress = raw;
    o->ai_damping = 1.0 / (1.0 + s->ai_capex);
    o->M8         = o->raw_stress * o->ai_damping;

    /* debt_pressure for dashboard */
    o->debt_pressure = clip01(s->debt_gdp / 200.0) * 0.5 +
                       clip01(s->int_rev / 30.0) * 0.5;

    if (o->M8 < 0.8) {
        strcpy(o->regime, "Calm");
        o->ponr_probability = 0.05;
    } else if (o->M8 < 1.5) {
        strcpy(o->regime, "Watch");
        o->ponr_probability = 0.20;
    } else {
        strcpy(o->regime, "Stress");
        o->ponr_probability = 0.50;
    }
}
