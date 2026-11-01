#ifndef RENDER_H
#define RENDER_H

#include <gsKit.h>
#include <tamtypes.h>
#include "game.h"

/* Global gsKit context (defined in render.c) */
extern GSGLOBAL *gsGlobal;

/* Initialize graphics (call once at startup) */
void init_gs(void);

/* Called every frame by main loop; handles state-based drawing
   and bufferswap/flip */
void render_frame(void);

/* Per-state draw functions (called by render_frame) */
void render_intro(void);
void render_reqs(void);
void render_setup(void);
void render_settings(void);
void render_game(void);

/* Intro timing (seconds since intro started); reset by init_gs() */
extern double g_intro_time;

#endif