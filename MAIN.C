/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <graph.h>
#include <conio.h>
#include <stdio.h>

#include "colors.h"
#include "state.h"
#include "system.h"
#include "modules.h"
#include "scenario.h"
#include "browser.h"
#include "filebro.h"
#include "dash.h"
#include "qxcalib.h"
#include "intro.h"


#define LOGO_LEFT      35
#define LOGO_COL       (LOGO_LEFT / 8)   /* 8px BIOS font width */

#define MENU_WIDTH   40   /* longest menu line */
#define MENU_COL     (LOGO_COL + ( (80 - LOGO_COL) - MENU_WIDTH ) / 2 )

#define MENU_HEIGHT   7
#define MENU_TOP      ((25 - MENU_HEIGHT) / 2)

int main(void)
{
    State         s;
    SystemOut     o;
    int           idx;
    int           ch;
    int           added;
    int           ok;
    char          filename[13];
    QXCalibResult r;
    double        lagged_ai = 0.0;

    _setvideomode(_TEXTC80);
    run_intro();

    for (;;) {
        _clearscreen(_GCLEARSCREEN);
        _settextcolor(COL_NORMAL);

        /* Title */
        _settextposition(MENU_TOP, MENU_COL);
        _outtext("QUANTXT v1.12 (IBM PC XT Edition)");

        /* Menu items */
        _settextposition(MENU_TOP + 2, MENU_COL);
        _settextcolor(COL_HEADER);
        _outtext("1) Browse scenarios");

        _settextposition(MENU_TOP + 3, MENU_COL);
        _settextcolor(COL_HEADER);
        _outtext("2) Manual input");

        _settextposition(MENU_TOP + 4, MENU_COL);
        _settextcolor(COL_HEADER);
        _outtext("3) Load scenario file");

        _settextposition(MENU_TOP + 5, MENU_COL);
        _settextcolor(COL_HEADER);
        _outtext("4) Run calibration file");

        _settextposition(MENU_TOP + 6, MENU_COL);
        _settextcolor(COL_HEADER);
        _outtext("ESC) Exit");

        /* Prompt */
        _settextposition(MENU_TOP + 8, MENU_COL);
        _settextcolor(COL_NORMAL);
        _outtext("Select option: ");



        ch = getch();

        if (ch == 27) {
            break;
        }
        else if (ch == '1') {
            idx = browse_scenarios();
            if (idx < 0) continue;
            fill_state_from_scenario(&scenarios[idx], &s, lagged_ai);
        }
        else if (ch == '2') {
            manual_input(&s);
        }
        else if (ch == '3') {
            if (browse_txt_files(filename)) {
                added = load_scenario_file_multi(filename);

                _clearscreen(_GCLEARSCREEN);
                _settextposition(1, 1);

                if (added > 0) {
                    _settextcolor(COL_HEADER);
                    _outtext("Scenarios loaded. Entering browser...");
                    _settextcolor(COL_NORMAL);

                    /* brief pause */
                    {
                        volatile long d;
                        for (d = 0; d < 200000L; d++);
                    }

                    idx = browse_scenarios();
                    if (idx >= 0) {
                        fill_state_from_scenario(&scenarios[idx], &s, lagged_ai);
                        compute_system_out(&s, &o);
                        draw_dashboard(&o, &s);
                        lagged_ai = s.ai_capex;
                    }
                } else {
                    _settextcolor(COL_WARNING);
                    _outtext("No scenarios found in file.");
                    _settextcolor(COL_NORMAL);
                    _settextposition(2, 1);
                    _outtext("Check format: name f1 f2 ... f15 per line");
                    _settextposition(3, 1);
                    _outtext("Press any key...");
                    getch();
                }
            }
            continue;
        }
        else if (ch == '4') {
            /* -------------------------------
            RUN CALIBRATION MODE
            ------------------------------- */
            if (!browse_txt_files(filename)) continue;

            /* --- in progress screen --- */
            _clearscreen(_GCLEARSCREEN);

            _settextcolor(COL_HEADER);
            _settextposition(MENU_TOP,     MENU_COL);
            _outtext("QUANTXT CALIBRATION");

            _settextcolor(COL_NORMAL);
            _settextposition(MENU_TOP + 1, MENU_COL);
            _outtext("----------------------------------------");

            _settextposition(MENU_TOP + 3, MENU_COL);
            _outtext("File : ");
            _outtext(filename);

            _settextcolor(COL_HEADER + 16);          /* blinking */
            _settextposition(MENU_TOP + 5, MENU_COL);
            _outtext("Calibration in progress...");

            _settextcolor(COL_NORMAL);
            _settextposition(MENU_TOP + 7, MENU_COL);
            _outtext("Please wait. Do not power off.");

            _settextposition(MENU_TOP + 10, MENU_COL);
            _outtext("----------------------------------------");

            /* --- run --- */
            ok = qxcalib_run(filename, "CALIB_RESULT.TXT", &r);

            /* --- result --- */
            if (ok == 0) {
                qxcalib_display(&r);
            } else {
                _clearscreen(_GCLEARSCREEN);
                _settextposition(1, 1);
                _settextcolor(COL_WARNING);
                _outtext("Calibration failed.");
                _settextcolor(COL_NORMAL);
                _settextposition(3, 1);
                printf("Could not open: %s\n", filename);
                printf("Check file format.\n");
                printf("\nPress any key...");
                getch();
            }
            continue;
        }
        else {
            continue;
        }

        compute_system_out(&s, &o);
        draw_dashboard(&o, &s);

        lagged_ai = s.ai_capex; /* simple lag example */
    }

    _setvideomode(_DEFAULTMODE);
    return 0;
}
