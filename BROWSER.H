/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef BROWSER_H
#define BROWSER_H

#include "state.h"

/*
 * browser.h — Scenario browser + file loader UI for IBM PC XT text mode
 *
 * Provides:
 *   - browse_scenarios(): scrollable list of built‑in + loaded scenarios
 *   - browse_txt_files(): DOS 8.3 file picker for .TXT scenario packs
 *
 * browse_scenarios()
 *   Returns:
 *      >= 0   index of selected scenario
 *      -1     user pressed ESC
 *
 * browse_txt_files()
 *   Arguments:
 *      out_filename — caller‑allocated buffer (char[128])
 *
 *   Returns:
 *      1  user selected a file (filename copied into out_filename)
 *      0  user cancelled
 *
 * These declarations intentionally avoid pulling in UI dependencies.
 */

int browse_scenarios(void);
int browse_txt_files(char *out_filename);

#endif
