/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <conio.h>
#include <graph.h>
#include <stdio.h>
#include "state.h"
#include "system.h"
#include "scenario.h"
#include "browser.h"
#include "colors.h"

#define BROWSER_TOP_ROW  3
#define BROWSER_VISIBLE  18   /* rows available below header/footer */

int browse_scenarios(void)
{
    int i;
    int ch;
    int sel = 0;
    int top = 0;   /* index of first visible scenario */

    for (;;) {
        int row = BROWSER_TOP_ROW;
        int visible_end;

        _clearscreen(_GCLEARSCREEN);
        _settextposition(1, 1);
        _settextcolor(COL_HEADER);
        _outtext("SCENARIO BROWSER  (UP/DOWN=scroll  ENTER=select  ESC=cancel)");

        /* scroll indicators */
        _settextposition(2, 1);
        _settextcolor(COL_NORMAL);
        if (top > 0)
            _outtext("  ^ more above");
        else
            _outtext("               ");

        visible_end = top + BROWSER_VISIBLE;
        if (visible_end > scenario_count)
            visible_end = scenario_count;

        for (i = top; i < visible_end; i++) {
            _settextposition(row + (i - top), 3);
            if (i == sel) {
                _settextcolor(COL_HIGHLIGHT);
                _outtext(">");
            } else {
                _settextcolor(COL_NORMAL);
                _outtext(" ");
            }
            _settextposition(row + (i - top), 5);
            _settextcolor(COL_NORMAL);
            _outtext(scenarios[i].name);
        }

        /* bottom scroll indicator */
        if (visible_end < scenario_count) {
            _settextposition(row + BROWSER_VISIBLE, 3);
            _settextcolor(COL_NORMAL);
            _outtext("  v more below");
        }

        /* counter */
        {
            char buf[24];
            sprintf(buf, "  %d/%d", sel + 1, scenario_count);
            _settextposition(row + BROWSER_VISIBLE + 1, 3);
            _settextcolor(COL_NORMAL);
            _outtext(buf);
        }

        ch = getch();
        if (ch == 0 || ch == 0xE0) {
            ch = getch();
            if (ch == 72) {                        /* UP */
                if (sel > 0) {
                    sel--;
                    if (sel < top)
                        top = sel;                 /* scroll window up */
                }
            } else if (ch == 80) {                 /* DOWN */
                if (sel < scenario_count - 1) {
                    sel++;
                    if (sel >= top + BROWSER_VISIBLE)
                        top = sel - BROWSER_VISIBLE + 1; /* scroll window down */
                }
            } else if (ch == 73) {                 /* PgUp */
                sel -= BROWSER_VISIBLE;
                if (sel < 0) sel = 0;
                top = sel;
            } else if (ch == 81) {                 /* PgDn */
                sel += BROWSER_VISIBLE;
                if (sel >= scenario_count) sel = scenario_count - 1;
                top = sel - BROWSER_VISIBLE + 1;
                if (top < 0) top = 0;
            }
        } else if (ch == 13) {
            return sel;
        } else if (ch == 27) {
            return -1;
        }
    }
}
