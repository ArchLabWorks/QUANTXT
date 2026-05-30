/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef SCENARIO_H
#define SCENARIO_H

#define MAX_SCENARIOS 256

#include "state.h"
#include "calibpar.h"

/*
 * scenario.h — Scenario registry + loader interface
 *
 * A Scenario is a full model snapshot:
 *   - name: scenario label
 *   - state: all macro-financial inputs (State)
 *   - params: calibrated model parameters (CalibParams)
 *
 * The loader appends new scenarios to the global registry and
 * increments scenario_count. It does NOT clear existing scenarios.
 *
 * This header intentionally avoids UI dependencies.
 */

/* ---------------------------------------------------------
 * Scenario struct — includes State + CalibParams
 * --------------------------------------------------------- */
typedef struct {
    char        name[32];
    State       state;
    CalibParams params;
} Scenario;

/* Global scenario registry */
extern Scenario scenarios[MAX_SCENARIOS];
extern int scenario_count;

/* Hydrate a State from a Scenario */
void fill_state_from_scenario(const Scenario *sc, State *s, double lagged_ai);

/* Manual entry UI */
void manual_input(State *s);

/* Multi‑scenario TXT loader */
int load_scenario_file_multi(const char *filename);

#endif /* SCENARIO_H */
