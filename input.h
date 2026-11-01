#ifndef INPUT_H
#define INPUT_H

#include <tamtypes.h>

/* Bitmask values returned by read_pad()
   (independent of PS2 PAD_* constants so main.c stays portable) */
#define BTN_LEFT   0x0001
#define BTN_RIGHT  0x0002
#define BTN_UP     0x0004
#define BTN_DOWN   0x0008
#define BTN_CROSS  0x0010
#define BTN_CIRCLE 0x0020
#define BTN_TRIANGLE 0x0040
#define BTN_SQUARE 0x0080
#define BTN_START  0x0100
#define BTN_SELECT 0x0200

/* Initialize controller port 0 (call once at startup) */
void init_pad(void);

/* Read the current button state of controller 0.
   Returns a bitmask of BTN_* values. Returns 0 if no pad is ready. */
int read_pad(void);

#endif