#pragma once

#include "../graphics/color.h"
#include "hil.h"
#include <stdbool.h>
#include <stdint.h>

#define TETRIS_ARENA_WIDTH 10
#define TETRIS_ARENA_HEIGHT 20
#define TETRIS_ARENA_X_OFFSET 2
#define TETRIS_ARENA_Y_OFFSET 3
#define TETRIS_ARENA_HELD_SIZE 6
#define TETRIS_MOVE_DELAY 30
#define TETRIS_BAG_SIZE 7
#define TETRIS_FULL_BAG_MASK ((1u << TETRIS_BAG_SIZE) - 1u)
#define TETRIS_KICKTEST_COUNT 5
#define TETRIS_DRAW_SCALE 3
#define TETRIS_CELL_SIZE (3 * TETRIS_DRAW_SCALE)
#define TETRIS_CLEAR_DELAY 60
#define TETRIS_GAMEOVER_DELAY 90
#define TETRIS_BLINK_DELAY 15
#define TETRIS_ROT_MASK 3u

typedef enum {
    TETRIS_NONE = -1,
    TETRIS_I_IDX = 0,
    TETRIS_J_IDX,
    TETRIS_L_IDX,
    TETRIS_Z_IDX,
    TETRIS_T_IDX,
    TETRIS_S_IDX,
    TETRIS_O_IDX,
} tetromino_id_t;

typedef struct {
    int8_t x;
    int8_t y;
    uint8_t r;
} tetro_pos_t;

typedef struct {
    const uint8_t *cells;
    uint8_t size;
} tetromino_def_t;

typedef struct {
    uint8_t arena[TETRIS_ARENA_HEIGHT][TETRIS_ARENA_WIDTH];
    uint8_t bag;

    uint8_t move_delay;
    uint8_t clear_delay;
    uint8_t game_over_delay;

    tetro_pos_t curr_pos;
    tetromino_id_t active_tetro;
    tetromino_id_t held_tetro;

    hil_button_state_t prev_key;

    bool has_tetromino;
    bool clearing_lines;
    bool is_blinking;
    bool game_over;
    bool full_rows[TETRIS_ARENA_HEIGHT];
} tetris_state_t;

static const uint8_t TETRIS_I[4][4] = {
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
};

static const uint8_t TETRIS_J[3][3] = {
    {1, 0, 0},
    {1, 1, 1},
    {0, 0, 0},
};

static const uint8_t TETRIS_L[3][3] = {
    {0, 0, 1},
    {1, 1, 1},
    {0, 0, 0},
};

static const uint8_t TETRIS_O[2][2] = {
    {1, 1},
    {1, 1},
};

static const uint8_t TETRIS_Z[3][3] = {
    {0, 1, 1},
    {1, 1, 0},
    {0, 0, 0},
};

static const uint8_t TETRIS_T[3][3] = {
    {0, 1, 0},
    {1, 1, 1},
    {0, 0, 0},
};

static const uint8_t TETRIS_S[3][3] = {
    {1, 1, 0},
    {0, 1, 1},
    {0, 0, 0},
};

static const tetromino_def_t TETRIS_BLOCKS[TETRIS_BAG_SIZE] = {
    [TETRIS_I_IDX] = {&TETRIS_I[0][0], 4},
    [TETRIS_J_IDX] = {&TETRIS_J[0][0], 3},
    [TETRIS_L_IDX] = {&TETRIS_L[0][0], 3},
    [TETRIS_Z_IDX] = {&TETRIS_Z[0][0], 3},
    [TETRIS_T_IDX] = {&TETRIS_T[0][0], 3},
    [TETRIS_S_IDX] = {&TETRIS_S[0][0], 3},
    [TETRIS_O_IDX] = {&TETRIS_O[0][0], 2},
};

static const color_t TETRIS_BLOCK_GRADIENT[3 * 3] = {
    0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0x98, 0x98, 0x98,
    0xa8, 0xa8, 0xa8, 0x98, 0x98, 0x98, 0x88, 0x88, 0x88,
    0x98, 0x98, 0x98, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
};

static const color_t TETRIS_COLORS[7] = {
    0x33, 0xd6, 0xff, // Cyan I
    0x3b, 0x6e, 0xff, // Blue J
    0xff, 0x99, 0x22, // Orange L
    0xf2, 0x38, 0x38, // Red Z
    0xb3, 0x47, 0xf5, // Purple T
    0x38, 0xd9, 0x59, // Green S
    0xff, 0xd9, 0x26, // Yellow O
};

static const color_t TETRIS_BORDER_COLOR = {0x4a, 0x52, 0x62};

static const color_t TETRIS_BLACK_COLOR = {0x00, 0x00, 0x00};

static const int8_t JLSTZ_CW_KICKS[4][5][2] = {
    [1] = {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}}, // 0 -> R
    [2] = {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}, // R -> 2
    [3] = {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}}, // 2 -> L
    [0] = {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}}, // L -> 0
};

static const int8_t I_CW_KICKS[4][5][2] = {
    [1] = {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2}}, // 0 -> R
    [2] = {{0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1}}, // R -> 2
    [3] = {{0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2}}, // 2 -> L
    [0] = {{0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1}}, // L -> 0
};

static const int8_t JLSTZ_CCW_KICKS[4][5][2] = {
    [0] = {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}, // R -> 0
    [1] = {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}}, // 2 -> R
    [2] = {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}}, // L -> 2
    [3] = {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}}, // 0 -> L
};

static const int8_t I_CCW_KICKS[4][5][2] = {
    [0] = {{0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2}}, // R -> 0
    [1] = {{0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1}}, // 2 -> R
    [2] = {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2}}, // L -> 2
    [3] = {{0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1}}, // 0 -> L
};