/* modload.c -- Load CalibParams from MODL.TXT
   C89 / DOS / Watcom compatible */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "calibpar.h"

/* ---------------------------------------------------------
 * Trim leading/trailing whitespace (C89-safe)
 * --------------------------------------------------------- */
static char *trim(char *s)
{
    char *end;

    while (*s==' ' || *s=='\t' || *s=='\r' || *s=='\n')
        s++;

    if (*s == '\0')
        return s;

    end = s + strlen(s) - 1;
    while (end > s &&
          (*end==' ' || *end=='\t' || *end=='\r' || *end=='\n'))
    {
        *end = '\0';
        end--;
    }

    return s;
}

/* ---------------------------------------------------------
 * Parse key=value line
 * --------------------------------------------------------- */
static int parse_kv(char *line, char **key, char **val)
{
    char *eq = strchr(line, '=');
    if (!eq)
        return 1;

    *eq = '\0';
    *key = trim(line);
    *val = trim(eq + 1);

    if (**key == '\0' || **val == '\0')
        return 1;

    return 0;
}

/* ---------------------------------------------------------
 * Load MODL.TXT → CalibParams
 * Returns 1 on success, 0 on failure
 * --------------------------------------------------------- */
int load_model_params(const char *filename, CalibParams *p)
{
    FILE *fp;
    char line[128];
    char *key;
    char *val;
    char *t;

    fp = fopen(filename, "r");
    if (!fp)
        return 0;

    while (fgets(line, sizeof(line), fp)) {

        t = trim(line);

        if (*t == '\0' || *t == '#' || *t == ';')
            continue;

        if (parse_kv(t, &key, &val) != 0)
            continue;

        if      (strcmp(key, "m1_w")            == 0) p->m1_w            = atof(val);
        else if (strcmp(key, "m3_w")            == 0) p->m3_w            = atof(val);
        else if (strcmp(key, "m8_mid")          == 0) p->m8_mid          = atof(val);
        else if (strcmp(key, "m8_k")            == 0) p->m8_k            = atof(val);
        else if (strcmp(key, "ai_coef")         == 0) p->ai_coef         = atof(val);
        else if (strcmp(key, "hysteresis")      == 0) p->hysteresis      = atof(val);
        else if (strcmp(key, "regime_penalty")  == 0) p->regime_penalty  = atof(val);
        else if (strcmp(key, "liquidity_floor") == 0) p->liquidity_floor = atof(val);
        else if (strcmp(key, "tail_risk_scale") == 0) p->tail_risk_scale = atof(val);

        /* Unknown keys ignored */
    }

    fclose(fp);
    return 1;
}
