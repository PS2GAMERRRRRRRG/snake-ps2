#ifndef FONT_H
#define FONT_H

#include <gsKit.h>
#include <tamtypes.h>

/* Font metrics: 5x7 pixel glyph, scaled 2x = 10x14 on screen */
#define FONT_CHAR_W  6    /* 5 columns + 1 spacing */
#define FONT_CHAR_H  8    /* 7 rows + 1 spacing */
#define FONT_SCALE   2
#define FONT_PIX_W   (FONT_CHAR_W * FONT_SCALE)   /* 12 px per char */
#define FONT_PIX_H   (FONT_CHAR_H * FONT_SCALE)   /* 16 px per char */

/* Draw a single ASCII char (32..126) at pixel position (x, y).
   Color is a gsKit RGBAQ value (use GS_SETREG_RGBAQ). */
void font_char(GSGLOBAL *gs, int x, int y, unsigned char c, u64 color);

/* Draw a printf-style string starting at (x, y). */
void font_printf(GSGLOBAL *gs, int x, int y, u64 color, const char *fmt, ...);

/* Draw a printf-style string horizontally centered in 640px screen. */
void font_printf_center(GSGLOBAL *gs, int y, u64 color, const char *fmt, ...);

/* Measure pixel width of a string (useful for right-align etc.) */
int  font_width(const char *s);

#endif