/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include "colors.h"
#include "filebro.h"

#define FILEBRO_TOP_ROW  4
#define FILEBRO_VISIBLE  16

char file_list[MAX_FILES][13];
int  file_count = 0;

/* ---------------------------------------------------------
 * Load all *.TXT files in the current directory
 * --------------------------------------------------------- */
void load_txt_file_list(void)
{
    struct find_t f;
    file_count = 0;

    if (_dos_findfirst("*.TXT", _A_NORMAL, &f) == 0) {
        do {
            if (file_count < MAX_FILES) {
                strncpy(file_list[file_count], f.name, 12);
                file_list[file_count][12] = '\0';
                file_count++;
            }
        } while (_dos_findnext(&f) == 0);
    }
}

/* ---------------------------------------------------------
 * XT-style scrollable file picker
 *
 * Returns:
 *   1 -> user selected a file (copied into out_filename)
 *   0 -> user cancelled (ESC) or no files found
 * --------------------------------------------------------- */
int browse_txt_files(char *out_filename)
{
    int sel = 0;
    int top = 0;
    int ch;
    int i;
    int visible_end;
    char buf[32];

    load_txt_file_list();

    if (file_count == 0) {
        _clearscreen(_GCLEARSCREEN);
        _settextcolor(COL_NORMAL);
        _settextposition(1, 1);
        _outtext("No .TXT files found in current directory.");
        _settextposition(2, 1);
        _outtext("Press any key...");
        getch();
        return 0;
    }

    for (;;) {
        _clearscreen(_GCLEARSCREEN);

        /* --- header --- */
        _settextcolor(COL_HEADER);
        _settextposition(1, 1);
        _outtext("SELECT SCENARIO FILE  (UP/DOWN=scroll  PGUP/PGDN=page  ENTER=load  ESC=cancel)");

        _settextposition(2, 1);
        _settextcolor(COL_NORMAL);
        _outtext("----------------------------------------");

        /* --- top scroll indicator --- */
        _settextposition(3, 1);
        if (top > 0) {
            _settextcolor(COL_FOOTER);
            _outtext("  ^ more above");
        } else {
            _outtext("               ");
        }

        /* --- visible file list --- */
        visible_end = top + FILEBRO_VISIBLE;
        if (visible_end > file_count)
            visible_end = file_count;

        for (i = top; i < visible_end; i++) {
            _settextposition(FILEBRO_TOP_ROW + (i - top), 4);
            if (i == sel) {
                _settextcolor(COL_HIGHLIGHT);
                _outtext("> ");
            } else {
                _settextcolor(COL_NORMAL);
                _outtext("  ");
            }
            _settextposition(FILEBRO_TOP_ROW + (i - top), 6);
            _settextcolor(COL_NORMAL);
            _outtext(file_list[i]);
        }

        /* --- bottom scroll indicator --- */
        _settextposition(FILEBRO_TOP_ROW + FILEBRO_VISIBLE, 1);
        if (visible_end < file_count) {
            _settextcolor(COL_FOOTER);
            _outtext("  v more below");
        } else {
            _outtext("               ");
        }

        /* --- counter --- */
        sprintf(buf, "  %d / %d", sel + 1, file_count);
        _settextposition(FILEBRO_TOP_ROW + FILEBRO_VISIBLE + 1, 1);
        _settextcolor(COL_FOOTER);
        _outtext(buf);

        /* --- key handling --- */
        ch = getch();
        if (ch == 0 || ch == 0xE0) {
            ch = getch();
            switch (ch) {
                case 72:                                   /* UP */
                    if (sel > 0) {
                        sel--;
                        if (sel < top)
                            top = sel;
                    }
                    break;
                case 80:                                   /* DOWN */
                    if (sel < file_count - 1) {
                        sel++;
                        if (sel >= top + FILEBRO_VISIBLE)
                            top = sel - FILEBRO_VISIBLE + 1;
                    }
                    break;
                case 73:                                   /* PgUp */
                    sel -= FILEBRO_VISIBLE;
                    if (sel < 0) sel = 0;
                    top = sel;
                    break;
                case 81:                                   /* PgDn */
                    sel += FILEBRO_VISIBLE;
                    if (sel >= file_count) sel = file_count - 1;
                    top = sel - FILEBRO_VISIBLE + 1;
                    if (top < 0) top = 0;
                    break;
            }
        } else if (ch == 13) {                             /* ENTER */
            strncpy(out_filename, file_list[sel], 12);
            out_filename[12] = '\0';
            return 1;
        } else if (ch == 27) {                             /* ESC */
            return 0;
        }
    }
}
