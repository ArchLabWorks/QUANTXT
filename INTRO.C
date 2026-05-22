/* Copyright (c) 2026 ArchLabWorks/ArchitectureLabs
   Licensed under the Apache License, Version 2.0.
   See the LICENSE file in the project root for details. */

#include <graph.h>
#include <bios.h>
#include <string.h>

/* ---------------------------------------------------------
   Layout parameters (logo + grid alignment)
   --------------------------------------------------------- */
#define LOGO_LEFT      35
#define LOGO_RIGHT     615
#define LOGO_WIDTH     (LOGO_RIGHT - LOGO_LEFT)

#define GRID_LEFT      LOGO_LEFT
#define GRID_RIGHT     LOGO_RIGHT
#define GRID_WIDTH     (GRID_RIGHT - GRID_LEFT)

/* Choose a column width that divides 580 exactly */
#define GRID_COL_W     29   /* or 20, 58, 116, etc. */

/* Center BIOS text horizontally in CGA Mode 6 */
static void center_text(int row, const char *s)
{
    int len = strlen(s);
    int col = (80 - len) / 2;   /* 80 columns in CGA text grid */
    _settextposition(row, col);
    _outtext((char __far *)s);  /* Watcom requires far pointer */
}

/* ---------------------------------------------------------
   Forward declarations
   --------------------------------------------------------- */
static void wait_ticks(unsigned long ticks);

/* ---------------------------------------------------------
   BIOS tick timing (cycle-accurate on XT, DOSBox, PCem)
   --------------------------------------------------------- */
static unsigned long read_ticks(void)
{
    long t;                          /* long *, as _bios_timeofday requires */
    _bios_timeofday(_TIME_GETCLOCK, &t);
    return (unsigned long)t;
}

static void wait_ticks(unsigned long ticks)
{
    unsigned long start = read_ticks();
    while ((read_ticks() - start) < ticks) {
        /* idle */
    }
}

/* ---------------------------------------------------------
   Draw QUANTXT logo using CGA vector strokes
   --------------------------------------------------------- */
static void draw_logo(void)
{
    int o, xo;

    for (o = 0; o < 3; o++) {
        _setcolor((o % 2) ? 1 : 2);  /* shimmer */

        for (xo = 0; xo < 2; xo++) {

            /* Q (width: 80px, tail offset: +100) */
            _moveto(LOGO_LEFT + xo,            40 + o);
            _lineto(LOGO_LEFT + 80 + xo,       40 + o);
            _lineto(LOGO_LEFT + 80 + xo,      120 + o);
            _lineto(LOGO_LEFT + xo,           120 + o);
            _lineto(LOGO_LEFT + xo,            40 + o);
            _moveto(LOGO_LEFT + 80 + xo,      120 + o);
            _lineto(LOGO_LEFT + 100 + xo,     140 + o);

            /* U (span: +120 → +200) */
            _moveto(LOGO_LEFT + 120 + xo,      40 + o);
            _lineto(LOGO_LEFT + 120 + xo,     120 + o);
            _lineto(LOGO_LEFT + 200 + xo,     120 + o);
            _lineto(LOGO_LEFT + 200 + xo,      40 + o);

            /* A (span: +220 → +300) */
            _moveto(LOGO_LEFT + 220 + xo,     120 + o);
            _lineto(LOGO_LEFT + 250 + xo,      40 + o);
            _lineto(LOGO_LEFT + 280 + xo,     120 + o);
            _moveto(LOGO_LEFT + 230 + xo,      90 + o);
            _lineto(LOGO_LEFT + 270 + xo,      90 + o);

            /* N (span: +300 → +340) */
            _moveto(LOGO_LEFT + 300 + xo,     120 + o);
            _lineto(LOGO_LEFT + 300 + xo,      40 + o);
            _lineto(LOGO_LEFT + 340 + xo,     120 + o);
            _lineto(LOGO_LEFT + 340 + xo,      40 + o);

            /* T (span: +360 → +440) */
            _moveto(LOGO_LEFT + 360 + xo,      40 + o);
            _lineto(LOGO_LEFT + 440 + xo,      40 + o);
            _moveto(LOGO_LEFT + 400 + xo,      40 + o);
            _lineto(LOGO_LEFT + 400 + xo,     120 + o);

            /* X (span: +460 → +500) */
            _moveto(LOGO_LEFT + 460 + xo,      40 + o);
            _lineto(LOGO_LEFT + 500 + xo,     120 + o);
            _moveto(LOGO_LEFT + 500 + xo,      40 + o);
            _lineto(LOGO_LEFT + 460 + xo,     120 + o);

            /* Final T (span: +520 → +580) */
            _moveto(LOGO_LEFT + 520 + xo,      40 + o);
            _lineto(LOGO_LEFT + 580 + xo,      40 + o);
            _moveto(LOGO_LEFT + 550 + xo,      40 + o);
            _lineto(LOGO_LEFT + 550 + xo,     120 + o);
        }
    }
}

/* ---------------------------------------------------------
   Draw 80s-style chart grid
   --------------------------------------------------------- */
static void draw_grid(void)
{
    int y;
    int x;

    /* horizontal grid lines */
    for (y = 150; y <= 190; y += 10) {
        _moveto(GRID_LEFT, y);
        _lineto(GRID_RIGHT, y);
    }

    /* vertical grid lines – exact tiling */
    for (x = GRID_LEFT; x <= GRID_RIGHT; x += GRID_COL_W) {
        _moveto(x, 150);
        _lineto(x, 190);
    }
}

/* ---------------------------------------------------------
   Scanline fade-in effect
   --------------------------------------------------------- */
static void scanline_fade(void)
{
    int y;
    for (y = 0; y < 200; y += 2) {
        _setcolor(1);
        _moveto(0, y);
        _lineto(639, y);
        wait_ticks(1); /* ~55 ms per scanline pair */
    }
}

/* ---------------------------------------------------------
   BIOS-font text
   --------------------------------------------------------- */
static void draw_text(void)
{
    _settextcolor(1);

    center_text(21, "QUANTXT v1.12 Macro-Risk Modeling Engine");
    center_text(22, "IBM PC/XT Compatible CGA Mode 6");
    center_text(23, "(C) 2026 QUANTXT Research Labs");
}


/* ---------------------------------------------------------
   Public intro entry point
   --------------------------------------------------------- */
void run_intro(void)
{
    _setvideomode(_HRESBW); /* 640x200 CGA mono */

    scanline_fade();
    draw_logo();
    wait_ticks(10);

    draw_grid();
    wait_ticks(10);

    draw_text();
    wait_ticks(40); /* hold for ~2 seconds */

    _setvideomode(_DEFAULTMODE);
}
