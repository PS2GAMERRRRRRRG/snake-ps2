#ifndef GAME_H
#define GAME_H

#include <tamtypes.h>

#define COLS 24
#define ROWS 16

typedef struct { int x, y; } Pt;
typedef struct { int startMs, stepMs, minMs, wrap, mult; } Diff;

typedef enum {
    ST_INTRO, ST_REQS, ST_SETUP, ST_SETTINGS, ST_PLAY, ST_PAUSED, ST_OVER
} GameState;

extern const Diff DIFFS[4];
extern Pt snake[COLS * ROWS];
extern int snake_len;
extern Pt dir, food;
extern int score, eaten, g_diff, g_hi[4];
extern GameState state;

void game_reset(void);
void game_start(void);
void tick(void);
void turn(int dx, int dy);
void update_game(u64 elapsed_us);
int  cur_interval_ms(void);

#endif
