#ifndef CALIBPAR_H
#define CALIBPAR_H

typedef struct CalibParamsTag {
    double m1_w;
    double m3_w;
    double m8_mid;
    double m8_k;
    double ai_coef;
    double hysteresis;
    double regime_penalty;
    double liquidity_floor;
    double tail_risk_scale;
} CalibParams;

#endif /* CALIBPAR_H */
