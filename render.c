#include "render.h"
#include "font.h"
#include <dmaKit.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>

#define SCREEN_W 640
#define SCREEN_H 448
#define CELL     20
#define MARGIN   40
#define HEADER   80

GSGLOBAL *gsGlobal = NULL;
double g_intro_time = 0.0;
static u64 C_BG = 0, C_GHOST = 0, C_DARK = 0;
static u64 THEMES[5][3];

static u64 rgba(int r, int g, int b) { return GS_SETREG_RGBAQ(r, g, b, 0x80, 0x00); }

static void build_themes(void) {
    const int T[5][9] = {
        { 164,191,70, 153,180,60, 28,42,18 }, { 120,190,215, 108,178,204, 10,35,60 },
        { 225,180,80, 212,167,68, 60,30,5 }, { 205,160,215, 192,148,203, 45,15,60 },
        { 200,200,190, 188,188,178, 35,35,40 }
    };
    for (int i = 0; i < 5; i++) {
        THEMES[i][0] = rgba(T[i][0], T[i][1], T[i][2]);
        THEMES[i][1] = rgba(T[i][3], T[i][4], T[i][5]);
        THEMES[i][2] = rgba(T[i][6], T[i][7], T[i][8]);
    }
}

static void apply_theme(int t) {
    if (t < 0) t = 0; if (t > 4) t = 4;
    C_BG = THEMES[t][0]; C_GHOST = THEMES[t][1]; C_DARK = THEMES[t][2];
}

static void rect(int x, int y, int w, int h, u64 color) {
    gsKit_prim_sprite(gsGlobal, x, y, x + w, y + h, 1, color);
}

/* ---- Init تنظیم شده برای PS2 ---- */
void init_gs(void) {
    gsGlobal = gsKit_init_global();
    gsGlobal->Mode   = GS_MODE_NTSC;
    gsGlobal->Width  = SCREEN_W;
    gsGlobal->Height = SCREEN_H;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsKit_init_screen(gsGlobal);
    
    /* حالت جدید: استفاده از PERSISTENT برای جلوگیری از مشکل صفحه خالی */
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT); 

    build_themes();
    apply_theme(0);
    g_intro_time = 0.0;

    gsKit_clear(gsGlobal, C_BG);
    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

/* ---- تست گرافیک ---- */
void render_intro(void) {
    apply_theme(0);
    gsKit_clear(gsGlobal, C_BG); 

    /* کشیدن 3 تا مربع بزرگ */
    rect(100, 100, 100, 100, C_DARK);
    rect(300, 100, 100, 100, C_DARK);
    rect(500, 100, 100, 100, C_DARK);

    /* کشیدن یک مار ساده */
    for(int i=0; i<5; i++) { rect(200 + (i*20), 250, 20, 20, C_DARK); }
    rect(300, 250, 20, 20, C_DARK);

    static u64 last = 0;
    static int first_frame = 1;
    u64 now = GetTimerSystemTime();
    if (!first_frame) {
        u64 diff = now - last;
        if (diff < 294912000ULL) { g_intro_time += (double)diff / 294912000.0; }
    }
    first_frame = 0;
    last = now;

    if (g_intro_time >= 3.0) { state = ST_REQS; g_intro_time = 0.0; }
}

void render_game(void) { gsKit_clear(gsGlobal, C_BG); }
void render_reqs(void) { gsKit_clear(gsGlobal, C_BG); }
void render_setup(void) { gsKit_clear(gsGlobal, C_BG); }
void render_settings(void) { gsKit_clear(gsGlobal, C_BG); }

void render_frame(void) {
    switch (state) {
        case ST_INTRO:  render_intro(); break;
        default:        gsKit_clear(gsGlobal, C_BG); break;
    }
    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}
