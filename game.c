#include "game.h"
#include <stdlib.h>

const Diff DIFFS[4] = {
    {190, 3, 110, 1, 1},
    {150, 5,  70, 0, 2},
    {110, 5,  55, 0, 3},
    { 80, 6,  40, 0, 5},
};

Pt snake[COLS * ROWS];
int snake_len = 0;
Pt dir = {1, 0}, food = {0, 0};
int score = 0, eaten = 0, g_diff = 1;
GameState state = ST_INTRO;
int g_hi[4] = {0, 0, 0, 0};

static u64 acc_us = 0;

static int same(Pt a, Pt b) { return a.x == b.x && a.y == b.y; }

int cur_interval_ms(void) {
    int v = DIFFS[g_diff].startMs - eaten * DIFFS[g_diff].stepMs;
    return v < DIFFS[g_diff].minMs ? DIFFS[g_diff].minMs : v;
}

static void spawn_food(void) {
    for (;;) {
        Pt p = { rand() % COLS, rand() % ROWS };
        int ok = 1;
        for (int i = 0; i < snake_len; i++)
            if (same(snake[i], p)) { ok = 0; break; }
        if (ok) { food = p; return; }
    }
}

void game_reset(void) {
    snake_len = 3;
    snake[0] = (Pt){6, ROWS / 2};
    snake[1] = (Pt){5, ROWS / 2};
    snake[2] = (Pt){4, ROWS / 2};
    dir = (Pt){1, 0};
    score = 0; eaten = 0; acc_us = 0;
    spawn_food();
}

void game_start(void) { game_reset(); state = ST_PLAY; }

static void game_over(void) {
    state = ST_OVER;
    if (score > g_hi[g_diff]) g_hi[g_diff] = score;
}

void tick(void) {
    Pt h = snake[0];
    Pt nh = { h.x + dir.x, h.y + dir.y };
    const Diff *d = &DIFFS[g_diff];

    if (d->wrap) {
        nh.x = (nh.x + COLS) % COLS;
        nh.y = (nh.y + ROWS) % ROWS;
    } else if (nh.x < 0 || nh.x >= COLS || nh.y < 0 || nh.y >= ROWS) {
        game_over(); return;
    }

    int eat = same(nh, food);
    for (int i = 0; i < snake_len; i++) {
        if (!eat && i == snake_len - 1) break;
        if (same(snake[i], nh)) { game_over(); return; }
    }

    for (int i = snake_len; i > 0; i--) snake[i] = snake[i - 1];
    snake[0] = nh;

    if (eat) {
        snake_len++; eaten++;
        score += 10 * d->mult;
        if (score > g_hi[g_diff]) g_hi[g_diff] = score;
        if (snake_len == COLS * ROWS) { game_over(); return; }
        spawn_food();
    }
}

void turn(int dx, int dy) {
    if (dx == -dir.x && dy == -dir.y) return;
    if (dx ==  dir.x && dy ==  dir.y) return;
    dir = (Pt){dx, dy};
}

void update_game(u64 elapsed_us) {
    if (state != ST_PLAY) return;
    acc_us += elapsed_us;
    int iv = cur_interval_ms() * 1000;
    int guard = 0;
    while (acc_us >= (u64)iv && state == ST_PLAY && guard++ < 5) {
        acc_us -= iv;
        tick();
        iv = cur_interval_ms() * 1000;
    }
}
