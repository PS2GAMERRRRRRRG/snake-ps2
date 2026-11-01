#include "input.h"
#include <libpad.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <tamtypes.h>
#include <string.h>

/* Pad state buffer (must be 64-byte aligned) */
static char pad_buf[256] __attribute__((aligned(64)));

/* Button status filled by padRead() */
static struct padButtonStatus buttons;

/* Set to 1 after padPortOpen succeeds */
static int pad_ready = 0;

/* ---- Init: load IOP modules and open port 0 ---- */
void init_pad(void) {
    /* IOP modules for controller support */
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN",  0, NULL);

    /* Init libpad and open controller port 0, slot 0 */
    padInit(0);
    padPortOpen(0, 0, pad_buf);

    pad_ready = 1;
}

/* ---- Read: return BTN_* bitmask of currently pressed buttons ---- */
int read_pad(void) {
    if (!pad_ready) return 0;

    int s = padGetState(0, 0);
    if (s != PAD_STATE_STABLE && s != PAD_STATE_FINDCTP1)
        return 0;   /* controller not ready */

    if (padRead(0, 0, &buttons) == 0)
        return 0;   /* read failed */

    /* buttons.btns has 0 = pressed, 1 = released -> invert */
    u16 b = 0xFFFF ^ buttons.btns;

    int r = 0;
    if (b & PAD_LEFT)     r |= BTN_LEFT;
    if (b & PAD_RIGHT)    r |= BTN_RIGHT;
    if (b & PAD_UP)       r |= BTN_UP;
    if (b & PAD_DOWN)     r |= BTN_DOWN;
    if (b & PAD_CROSS)    r |= BTN_CROSS;
    if (b & PAD_CIRCLE)   r |= BTN_CIRCLE;
    if (b & PAD_TRIANGLE) r |= BTN_TRIANGLE;
    if (b & PAD_SQUARE)   r |= BTN_SQUARE;
    if (b & PAD_START)    r |= BTN_START;
    if (b & PAD_SELECT)   r |= BTN_SELECT;

    return r;
}