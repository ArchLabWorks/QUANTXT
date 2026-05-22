/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include "history.h"

void history_reset(History *h)
{
    int i;
    h->count = 0;
    for (i = 0; i < HISTORY_MAX; i++) {
        h->m8[i]  = 0.0;
        h->m9[i]  = 0.0;
        h->m10[i] = 0.0;
    }
}

void history_append(History *h, double m8, double m9, double m10)
{
    if (h->count >= HISTORY_MAX)
        return;

    h->m8[h->count]  = m8;
    h->m9[h->count]  = m9;
    h->m10[h->count] = m10;
    h->count++;
}
