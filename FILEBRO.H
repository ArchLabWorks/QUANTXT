/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#ifndef FILEBRO_H
#define FILEBRO_H

#include "state.h"   /* Only needed if you reference Scenario names */

/*
 * filebro.h — DOS 8.3 TXT file browser for QUANTXT
 *
 * Provides:
 *   - load_txt_file_list(): populates file_list[] with *.TXT files
 *   - browse_txt_files(): scrollable picker that returns a filename
 *
 * browse_txt_files()
 *   Arguments:
 *      out_filename — caller-allocated buffer (char[128])
 *
 *   Returns:
 *      1  user selected a file (filename copied into out_filename)
 *      0  user cancelled
 *
 * Globals:
 *   file_list[MAX_FILES][13] — 8.3 filenames
 *   file_count               — number of TXT files found
 */

#define MAX_FILES 64

extern char file_list[MAX_FILES][13];
extern int  file_count;

/* Updated signature — REQUIRED by main.c and browser.c */
int browse_txt_files(char *out_filename);

/* Scans current directory for *.TXT files */
void load_txt_file_list(void);

#endif
