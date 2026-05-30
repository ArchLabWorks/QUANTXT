/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef ENGINE_H
#define ENGINE_H

#include "state.h"
#include "calibpar.h"

/*
 * Core QUANTXT evaluation function.
 * Takes a fully-populated State and a CalibParams block,
 * and returns a double score in [0, 1].
 */
double run_engine(const State *st, const CalibParams *p);

#endif /* ENGINE_H */
