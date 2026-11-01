#include "render.h"
#include "font.h"
#include <dmaKit.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>

/* ---- Screen & layout ---- */
#define SCREEN_W 640
#define SCREEN_H 448
#define CELL     20
#define MARGIN   40
#define HEADER   80

/* ---- gsKit global ---- */
GSGLOBAL *gsGlobal = NULL;

/* Intro timer (seconds) */
double g_intro_time = 0.0;

/* Current theme colors (updated by apply_theme) */
static u64 C_BG    = 0;
static u64 C_GHOST = 0;
static u64 C_DARK  = 0;

/* Theme table: 5 themes x 3 colors (bg, ghost, dark) */
static u64 THEMES[5][3];

static u64 rgba(int r, int g, int b) {
    return GS_SETREG_RGBAQ(r, g, b, 0x80, 0x00);
}

static void build_themes(void) {
    const int T[5][9] = {
        /*  bg R,G,B      ghost R,G,B    dark R,G,B  */
        { 164,191,70,     153,180,60,     28, 42, 18 },   /* classic green */
        { 120,190,215,    108,178,204,    10, 35, 60 },   /* ice blue      */
        { 225,180,80,     212,167,68,     60, 30,  5 },   /* amber         */
        { 205,160,215,    192,148,203,    45, 15, 60 },   /* purple        */
        { 200,200,190,    188,188,178,    35, 35, 40 }    /* grey          */
    };
    for (int i = 0; i < 5; i++) {
        THEMES[i][0] = rgba(T[i][0], T[i][1], T[i][2]);
        THEMES[i][1] = rgba(T[i][3], T[i][4], T[i][5]);
        THEMES[i][2] = rgba(T[i][6], T[i][7], T[i][8]);
    }
}

static void apply_theme(int t) {
    if (t < 0) t = 0;
    if (t > 4) t = 4;
    C_BG    = THEMES[t][0];
    C_GHOST = THEMES[t][1];
    C_DARK  = THEMES[t][2];
}

/* ---- Primitives ---- */
static void rect(int x, int y, int w, int h, u64 color) {
    gsKit_prim_sprite(gsGlobal, x, y, x + w, y + h, 1, color);
}

/* ---- Init ---- */
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
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

    build_themes();
    apply_theme(0);
    g_intro_time = 0.0;

    gsKit_clear(gsGlobal, C_BG);
    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

/* ---- Game field drawing ---- */
static void draw_cell(int cx, int cy) {
    int px = MARGIN + cx * CELL;
    int py = HEADER + cy * CELL;
    rect(px + 1, py + 1, CELL - 2, CELL - 2, C_DARK);
}

static void draw_food(int cx, int cy) {
    int px = MARGIN + cx * CELL + 1;
    int py = HEADER + cy * CELL + 1;
    const int u = 6;
    int pos[4][2] = { {1,0}, {0,1}, {2,1}, {1,2} };
    for (int i = 0; i < 4; i++)
        rect(px + pos[i][0] * u, py + pos[i][1] * u, u - 1, u - 1, C_DARK);
}

static void draw_head(int cx, int cy, Pt d) {
    draw_cell(cx, cy);
    int px = MARGIN + cx * CELL;
    int py = HEADER + cy * CELL;
    int ex[2], ey[2];
    if (d.x == 1)       { ex[0] = ex[1] = 12; ey[0] = 4;  ey[1] = 12; }
    else if (d.x == -1) { ex[0] = ex[1] = 4;  ey[0] = 4;  ey[1] = 12; }
    else if (d.y == -1) { ey[0] = ey[1] = 4;  ex[0] = 4;  ex[1] = 12; }
    else                { ey[0] = ey[1] = 12; ex[0] = 4;  ex[1] = 12; }
    for (int i = 0; i < 2; i++)
        rect(px + ex[i], py + ey[i], 4, 4, C_BG);
}

/* ---- Screen: Game ---- */
void render_game(void) {
    /* Theme changes every 500 points while playing */
    int theme = 0;
    if (state == ST_PLAY || state == ST_PAUSED || state == ST_OVER)
        theme = (score / 500) % 5;
    apply_theme(theme);
    gsKit_clear(gsGlobal, C_BG);

    /* Faint LCD grid pixels */
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            rect(MARGIN + x * CELL + 7, HEADER + y * CELL + 7, 6, 6, C_GHOST);

    /* Playfield frame */
    rect(MARGIN - 4, HEADER - 4, COLS * CELL + 8, ROWS * CELL + 8, C_DARK);
    rect(MARGIN - 1, HEADER - 1, COLS * CELL + 2, ROWS * CELL + 2, C_BG);

    /* Food */
    draw_food(food.x, food.y);

    /* Snake body */
    for (int i = 1; i < snake_len; i++)
        draw_cell(snake[i].x, snake[i].y);

    /* Snake head with eyes */
    if (snake_len > 0)
        draw_head(snake[0].x, snake[0].y, dir);

    /* Header text */
    font_printf(gsGlobal, MARGIN, 10, C_DARK, "SCORE %04d", score);
    font_printf(gsGlobal, SCREEN_W - MARGIN - 140, 10, C_DARK, "HI %04d", g_hi[g_diff]);

    /* Pause / Game Over overlays */
    if (state == ST_PAUSED) {
        rect(160, 170, 320, 110, C_DARK);
        rect(164, 174, 312, 102, C_BG);
        font_printf_center(gsGlobal, 195, C_DARK, "PAUSED");
        font_printf_center(gsGlobal, 235, C_DARK, "CROSS = CONTINUE");
        font_printf_center(gsGlobal, 260, C_DARK, "START = MENU");
    } else if (state == ST_OVER) {
        rect(120, 150, 400, 150, C_DARK);
        rect(124, 154, 392, 142, C_BG);
        font_printf_center(gsGlobal, 175, C_DARK, "GAME OVER");
        char buf[64];
        snprintf(buf, sizeof(buf), "SCORE %d", score);
        font_printf_center(gsGlobal, 215, C_DARK, buf);
        if (score > 0 && score >= g_hi[g_diff])
            font_printf_center(gsGlobal, 240, C_DARK, "NEW BEST!");
        font_printf_center(gsGlobal, 270, C_DARK, "CROSS = RETRY   START = MENU");
    }
}

/* ---- Screen: Intro ---- */
void render_intro(void) {
    apply_theme(0);
    gsKit_clear(gsGlobal, C_BG);

    font_printf_center(gsGlobal, 30,  C_DARK, "SNAKE");
    font_printf_center(gsGlobal, 75,  C_DARK, "NOKIA STYLE - REMASTERED");
    font_printf_center(gsGlobal, 105, C_DARK, "MADE BY TAHA ARDELANI");

    /* Social links */
    font_printf(gsGlobal, 124, 155, C_DARK, "YouTube: @TAHA_GAME15");
    font_printf(gsGlobal, 124, 185, C_DARK, "Telegram: @OLD_GAME_PE");
    font_printf(gsGlobal, 124, 215, C_DARK, "Support: @TAHA_RETRO");

    font_printf_center(gsGlobal, 265, C_DARK, "VERSION 0.6.0");

    /* Loading bar */
    int pct = (int)(g_intro_time / 5.0 * 100.0);
    if (pct > 100) pct = 100;
    if (pct < 0)   pct = 0;

    int bx = MARGIN + 60;
    int bw = SCREEN_W - 2 * MARGIN - 120;
    rect(bx, 315, bw, 18, C_DARK);
    rect(bx + 2, 317, (bw - 4) * pct / 100, 14, C_BG);

    char buf[32];
    snprintf(buf, sizeof(buf), "LOADING %d%%", pct);
    font_printf_center(gsGlobal, 295, C_DARK, buf);
}

/* ---- Screen: System Requirements ---- */
void render_reqs(void) {
    apply_theme(0);
    gsKit_clear(gsGlobal, C_BG);

    font_printf_center(gsGlobal, 20, C_DARK, "SYSTEM REQUIREMENTS");

    font_printf(gsGlobal, 180, 70, C_DARK, "MINIMUM");
    font_printf(gsGlobal, 400, 70, C_DARK, "RECOMMENDED");
    rect(40, 92, 560, 2, C_DARK);

    const char *labels[6] = { "OS", "CPU", "RAM", "GPU", "DISPLAY", "AUDIO" };
    const char *mn[6] = {
        "PS2 (any model)", "Emotion Engine", "32 MB", "Graphics Synthesizer",
        "NTSC 640x448", "Optional"
    };
    const char *rc[6] = {
        "PS2 + FMCB", "294 MHz", "32 MB", "GS + gsKit",
        "NTSC/PAL", "Any"
    };

    for (int i = 0; i < 6; i++) {
        int y = 100 + i * 34;
        if (i % 2 == 0)
            rect(40, y - 2, 560, 32, C_GHOST);
        font_printf(gsGlobal, 48,  y + 6, C_DARK, labels[i]);
        font_printf(gsGlobal, 200, y + 6, C_DARK, mn[i]);
        font_printf(gsGlobal, 400, y + 6, C_DARK, rc[i]);
    }

    font_printf_center(gsGlobal, 340, C_DARK, "YOUR CONSOLE MEETS RECOMMENDED SPECS");
    font_printf_center(gsGlobal, 390, C_DARK, "CROSS = CONTINUE");
}

/* ---- Screen: Setup ---- */
void render_setup(void) {
    apply_theme(0);
    gsKit_clear(gsGlobal, C_BG);

    font_printf_center(gsGlobal, 30, C_DARK, "SNAKE");

    const char *diff_names[4] = { "EASY", "NORMAL", "HARD", "INSANE" };

    font_printf(gsGlobal, 40, 100, C_DARK, "SNAKE NAME:");
    font_printf(gsGlobal, 260, 100, C_DARK, "PLAYER");

    font_printf(gsGlobal, 40, 145, C_DARK, "DIFFICULTY:");
    font_printf(gsGlobal, 260, 145, C_DARK, diff_names[g_diff]);

    font_printf(gsGlobal, 40, 200, C_DARK, "UP/DOWN = CHANGE DIFFICULTY");
    font_printf(gsGlobal, 40, 230, C_DARK, "CROSS = START GAME");
    font_printf(gsGlobal, 40, 260, C_DARK, "START = EXIT TO ULAUNCH");

    char buf[80];
    snprintf(buf, sizeof(buf), "START: %dms   FASTEST: %dms",
             DIFFS[g_diff].startMs, DIFFS[g_diff].minMs);
    font_printf_center(gsGlobal, 320, C_DARK, buf);

    snprintf(buf, sizeof(buf), "WALLS: %s   POINTS: x%d   BEST: %04d",
             DIFFS[g_diff].wrap ? "WRAP" : "SOLID",
             DIFFS[g_diff].mult, g_hi[g_diff]);
    font_printf_center(gsGlobal, 350, C_DARK, buf);
}

/* ---- Screen: Settings ---- */
void render_settings(void) {
    apply_theme(0);
    gsKit_clear(gsGlobal, C_BG);

    font_printf_center(gsGlobal, 30, C_DARK, "SETTINGS");

    font_printf(gsGlobal, 40, 110, C_DARK, "SOUND:          ON");
    font_printf(gsGlobal, 40, 150, C_DARK, "SAVE HI-SCORES: YES");
    font_printf(gsGlobal, 40, 190, C_DARK, "STORAGE:        MEMORY CARD (mc0:)");

    font_printf_center(gsGlobal, 320, C_DARK, "CROSS = BACK");
}

/* ---- Master render (called every frame) ---- */
void render_frame(void) {
    static u64 last = 0;
    static int first_frame = 1;

    u64 now = GetTimerSystemTime();
    if (!first_frame) {
        u64 diff = now - last;
        /* محافظ: اگر اختلاف زمان غیرعادی بود (مثلا برای شبیه‌ساز)، نادیده بگیر */
        if (diff < 294912000ULL) { /* کمتر از 1 ثانیه */
            g_intro_time += (double)diff / 294912000.0;
        }
    }
    first_frame = 0;
    last = now;

    switch (state) {
        case ST_INTRO:
            render_intro();
            if (g_intro_time >= 5.0) {
                state = ST_REQS;
                g_intro_time = 0.0;
            }
            break;

        case ST_REQS:
            render_reqs();
            break;

        case ST_SETUP:
            render_setup();
            break;

        case ST_SETTINGS:
            render_settings();
            break;

        case ST_PLAY:
        case ST_PAUSED:
        case ST_OVER:
            render_game();
            break;

        default:
            break;
    }

    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}
