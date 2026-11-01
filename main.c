#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <timer.h>
#include <stdlib.h>

#include "game.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "save.h"

/* PS2 timer tick = 294912000 per second (NTSC) */
#ifndef kTIME_ONE_SECOND
#define kTIME_ONE_SECOND 294912000ULL
#endif

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* Init PS2 subsystems */
    SifInitRpc(0);
    SifLoadFileInit();

    /* Init graphics, controller, sound, save */
    init_gs();
    init_pad();
    sound_init();
    save_load_hiscore(g_hi);

    /* Seed RNG with timer so food positions differ each boot */
    srand((unsigned int)GetTimerSystemTime());

    /* Start at the intro screen */
    game_reset();
    state = ST_INTRO;

    u64 last = GetTimerSystemTime();
    int last_btn = 0;

    while (1) {
        int btn = read_pad();

        /* Edge detection: only act on newly pressed buttons */
        int pressed = btn & ~last_btn;
        last_btn = btn;

        /* START quits back to uLaunchELF (except during gameplay) */
        if ((pressed & BTN_START) &&
            (state == ST_INTRO || state == ST_REQS || state == ST_SETUP ||
             state == ST_SETTINGS || state == ST_PAUSED)) {
            if (state == ST_PAUSED) {
                state = ST_SETUP;
            } else {
                break;  /* exit to uLaunchELF */
            }
        }

        switch (state) {
            case ST_INTRO:
                /* auto-advance is handled inside render_frame() */
                break;

            case ST_REQS:
                if (pressed & BTN_CROSS) state = ST_SETUP;
                break;

            case ST_SETUP:
                if (pressed & BTN_UP)    g_diff = (g_diff + 3) % 4;
                if (pressed & BTN_DOWN)  g_diff = (g_diff + 1) % 4;
                if (pressed & BTN_CROSS) game_start();
                break;

            case ST_SETTINGS:
                if (pressed & BTN_CROSS) state = ST_SETUP;
                break;

            case ST_PLAY:
                if (btn & BTN_LEFT)  turn(-1, 0);
                if (btn & BTN_RIGHT) turn( 1, 0);
                if (btn & BTN_UP)    turn(0, -1);
                if (btn & BTN_DOWN)  turn(0,  1);
                if (pressed & BTN_CROSS) state = ST_PAUSED;
                if (pressed & BTN_START) state = ST_SETUP;
                break;

            case ST_PAUSED:
                if (pressed & BTN_CROSS) state = ST_PLAY;
                if (pressed & BTN_START) state = ST_SETUP;
                break;

            case ST_OVER:
                save_store_hiscore(g_hi);
                if (pressed & BTN_CROSS) game_start();
                if (pressed & BTN_START) state = ST_SETUP;
                break;

            default:
                break;
        }

        /* Compute elapsed time in microseconds */
        u64 now = GetTimerSystemTime();
        u64 elapsed = now - last;
        last = now;
        u64 elapsed_us = (elapsed * 1000000ULL) / kTIME_ONE_SECOND;

        /* Update game logic */
        update_game(elapsed_us);

        /* Draw + flip */
        render_frame();
    }

    /* Clean exit */
    save_store_hiscore(g_hi);
    sound_stop();
    return 0;
}