#pragma once

#include "../graphics/color.h"
#include "hil.h"
#include <stdbool.h>

#define TETRIS_ARENA_WIDTH 10
#define TETRIS_ARENA_HEIGHT 20
#define TETRIS_MOVE_DELAY 20
#define TETRIS_BAG_SIZE 7
#define TETRIS_KICKTEST_COUNT 5

typedef enum {
    TETRIS_I_IDX = 0,
    TETRIS_J_IDX,
    TETRIS_L_IDX,
    TETRIS_Z_IDX,
    TETRIS_T_IDX,
    TETRIS_S_IDX,
    TETRIS_O_IDX,
} TETROMINOES;

typedef struct {
    int8_t x;
    int8_t y;
    uint8_t r;
} tet_pos_t;

static const color_t TETRIS_BLOCK_GRADIENT[3*3] = {
    0x2f,0x2f,0x2f,0x2f,0x2f,0x2f,0x2b,0x2b,0x2b,
    0x2f,0x2f,0x2f,0x2b,0x2b,0x2b,0x27,0x27,0x27,
    0x2b,0x2b,0x2b,0x27,0x27,0x27,0x27,0x27,0x27,
};

static const color_t TETRIS_COLORS[7] = {
    0x00, 0xff, 0xff, //cyan I
    0x00, 0x00, 0xff, //blue J
    0xff, 0xa5, 0x00, //orange L
    0xff, 0x00, 0x00, //red Z
    0x9d, 0x00, 0xff, //purple T
    0x00, 0xff, 0x00, //green S
    0xff, 0xff, 0x00, //yellow O
};

static const uint8_t TETRIS_I[4][4] = {
    {0,0,0,0},
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0},
};

static const uint8_t TETRIS_J[3][3] = {
    {1,0,0},
    {1,1,1},
    {0,0,0},
};

static const uint8_t TETRIS_L[3][3] = {
    {0,0,1},
    {1,1,1},
    {0,0,0},
};

static const uint8_t TETRIS_O[2][2] = {
    {1,1},
    {1,1},
};

static const uint8_t TETRIS_Z[3][3] = {
    {0,1,1},
    {1,1,0},
    {0,0,0},
};

static const uint8_t TETRIS_T[3][3] = {
    {0,1,0},
    {1,1,1},
    {0,0,0},
};

static const uint8_t TETRIS_S[3][3] = {
    {1,1,0},
    {0,1,1},
    {0,0,0},
};

typedef struct {
    const uint8_t *cells;
    uint8_t size;
} tetromino_def_t;

static const tetromino_def_t TETRIS_BLOCKS[TETRIS_BAG_SIZE] = {
    [TETRIS_I_IDX] = { &TETRIS_I[0][0], 4 },
    [TETRIS_J_IDX] = { &TETRIS_J[0][0], 3 },
    [TETRIS_L_IDX] = { &TETRIS_L[0][0], 3 },
    [TETRIS_Z_IDX] = { &TETRIS_Z[0][0], 3 },
    [TETRIS_T_IDX] = { &TETRIS_T[0][0], 3 },
    [TETRIS_S_IDX] = { &TETRIS_S[0][0], 3 },
    [TETRIS_O_IDX] = { &TETRIS_O[0][0], 2 },
};

static const int8_t JLSTZ_CW_KICKS[4][5][2] = {
    [1] = { {0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2} }, // 0 -> R
    [2] = { {0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2} }, // R -> 2
    [3] = { {0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2} }, // 2 -> L
    [0] = { {0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2} }, // L -> 0
};

static const int8_t I_CW_KICKS[4][5][2] = {
    [1] = { {0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2} }, // 0 -> R
    [2] = { {0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1} }, // R -> 2
    [3] = { {0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2} }, // 2 -> L
    [0] = { {0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1} }, // L -> 0
};

static const int8_t JLSTZ_CCW_KICKS[4][5][2] = {
    [0] = { {0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2} }, // R -> 0
    [1] = { {0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2} }, // 2 -> R
    [2] = { {0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2} }, // L -> 2
    [3] = { {0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2} }, // 0 -> L
};

static const int8_t I_CCW_KICKS[4][5][2] = {
    [0] = { {0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2} }, // R -> 0
    [1] = { {0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1} }, // 2 -> R
    [2] = { {0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2} }, // L -> 2
    [3] = { {0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1} }, // 0 -> L
};

void step(hil_button_state_t* btn_state);

void draw();

void rotate_clockwise(tet_pos_t* pos);

void rotate_anticlockwise(tet_pos_t* pos);

void drop(tet_pos_t* pos);

void move_left(tet_pos_t* pos);

void move_right(tet_pos_t* pos);

bool move_down(tet_pos_t* pos);

void lock_tetromino(TETROMINOES* curr, tet_pos_t* pos);

void clear_lines();

void handle_input(hil_button_state_t* btn_state);

void handle_game_over();

void spawn_tetromino(TETROMINOES* curr, tet_pos_t* pos);

void get_new_bag();

TETROMINOES get_tetromino();

bool is_bag_empty();

bool is_valid_position(TETROMINOES* curr, tet_pos_t* pos);

uint8_t get_rotated(const tetromino_def_t* block, uint8_t rot, uint8_t x, uint8_t y);