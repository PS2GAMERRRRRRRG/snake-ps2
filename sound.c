#include "sound.h"
#include <audsrv.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <tamtypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RATE 22050          /* sample rate (Hz) */
#define DING_SAMPLES (RATE * 2 / 5)   /* 0.4 seconds, stereo int16 */

/* Buffers (stereo int16 = 2 shorts per frame) */
static short ding_buf[DING_SAMPLES * 2];
static int   sound_ok = 0;
static int   muted = 0;

/* ---- Init: load IOP modules, open audsrv, build ding buffer ---- */
void sound_init(void) {
    sound_ok = 0;

    /* Try to load audsrv (IRX should be on USB next to the ELF,
       fallback to rom0 in case a modchip/freehdd has it there). */
    int ret = SifLoadModule("host:audsrv.irx", 0, NULL);
    if (ret < 0) ret = SifLoadModule("rom0:LIBSD", 0, NULL);
    if (ret < 0) return;

    if (audsrv_init() != 0) return;

    /* S16 stereo at 22 kHz */
    struct audsrv_fmt_t fmt;
    fmt.bits = 16;
    fmt.freq = RATE;
    fmt.channels = 2;
    audsrv_set_format(&fmt);
    
    audsrv_set_volume(80);

    /* ---- Build the ding: two short tones, bell-like envelope ---- */
    memset(ding_buf, 0, sizeof(ding_buf));

    /* Tone 1: 1318.5 Hz (E6), 0.0 - 0.12 s */
    int s0 = 0;
    int s1 = RATE * 12 / 100;
    for (int i = s0; i < s1; i++) {
        double t = (double)i / RATE;
        double attack = (i < 80) ? (i / 80.0) : 1.0;
        double env = exp(-t * 18.0) * attack;
        double v = sin(2.0 * M_PI * 1318.5 * t) * env;
        short smp = (short)(v * 14000);
        ding_buf[i * 2]     = smp;
        ding_buf[i * 2 + 1] = smp;
    }

    /* Tone 2: 1760.0 Hz (A6), 0.12 - 0.40 s */
    s0 = RATE * 12 / 100;
    s1 = RATE * 40 / 100;
    for (int i = s0; i < s1; i++) {
        double t = (double)(i - s0) / RATE;
        double env = exp(-t * 10.0);
        double v = sin(2.0 * M_PI * 1760.0 * t) * env;
        short smp = (short)(v * 14000);
        ding_buf[i * 2]     = smp;
        ding_buf[i * 2 + 1] = smp;
    }

    sound_ok = 1;
}

/* ---- Play ding (non-blocking) ---- */
void sound_play_ding(void) {
    if (!sound_ok || muted) return;
    audsrv_play_audio((char *)ding_buf, sizeof(ding_buf));
}

/* ---- Music: placeholder (no background track yet) ---- */
void sound_play_music(void) {
    /* Reserved for future looping music.
       For now, nothing plays in the background. */
}

/* ---- Stop ---- */
void sound_stop(void) {
    if (!sound_ok) return;
    audsrv_stop_audio();
}

/* ---- Mute toggle ---- */
void sound_toggle_mute(void) {
    muted = !muted;
    if (!sound_ok) return;
    audsrv_set_volume(muted ? 0 : 80);
}

int sound_is_muted(void) {
    return muted;
}
