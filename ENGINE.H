/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef ENGINE_H
#define ENGINE_H

#include "state.h"

/*
 * Core QUANTXT evaluation function.
 * Takes a fully-populated State and returns a float score.
 */
double run_engine(const State *st);

#endif
