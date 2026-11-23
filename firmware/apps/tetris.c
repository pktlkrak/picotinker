#include "tetris.h"
#include "tetris_data.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static tetris_state_t state;

static uint8_t get_rotated(const tetromino_def_t *block, uint8_t rot, uint8_t x,
                           uint8_t y) {
    if (x >= block->size || y >= block->size)
        return 0;

    uint8_t src_x, src_y;
    switch (rot) {
    case 0:
        src_x = x;
        src_y = y;
        break;
    case 1:
        src_x = y;
        src_y = block->size - 1 - x;
        break;
    case 2:
        src_x = block->size - 1 - x;
        src_y = block->size - 1 - y;
        break;
    case 3:
        src_x = block->size - 1 - y;
        src_y = x;
        break;
    default:
        src_x = 0;
        src_y = 0;
        break;
    }

    return block->cells[src_y * block->size + src_x];
}

static bool is_valid_position(const tetro_pos_t *pos) {
    const tetromino_def_t *block = &TETRIS_BLOCKS[state.active_tetro];

    for (uint8_t y = 0; y < block->size; y++) {
        for (uint8_t x = 0; x < block->size; x++) {
            uint8_t cell = get_rotated(block, pos->r, x, y);

            if (cell == 0)
                continue;

            int8_t arena_x = pos->x + x;
            int8_t arena_y = pos->y + y;

            if (arena_x < 0 || arena_x >= TETRIS_ARENA_WIDTH ||
                arena_y >= TETRIS_ARENA_HEIGHT) {
                return false;
            }

            if (arena_y < 0)
                continue;

            if (state.arena[arena_y][arena_x] != 0) {
                return false;
            }
        }
    }

    return true;
}

static void draw_cell(uint8_t *buffer, uint8_t x, uint8_t y, color_t color) {
    uint8_t start_x = x * TETRIS_CELL_SIZE;
    uint8_t start_y = y * TETRIS_CELL_SIZE;

    for (uint8_t dy = 0; dy < TETRIS_CELL_SIZE; dy++) {
        for (uint8_t dx = 0; dx < TETRIS_CELL_SIZE; dx++) {
            uint8_t screen_x = start_x + dx;
            uint8_t screen_y = start_y + dy;

            if (screen_x < SCREEN_SIZE_X && screen_y < SCREEN_SIZE_Y) {
                size_t index =
                    ((size_t)screen_y * SCREEN_SIZE_X + screen_x) * 3;
                uint8_t grad_x = dx / TETRIS_DRAW_SCALE;
                uint8_t grad_y = dy / TETRIS_DRAW_SCALE;
                color_t grad = TETRIS_BLOCK_GRADIENT[grad_y * 3 + grad_x];
                buffer[index + 0] = (color.r * grad.r) / 255u;
                buffer[index + 1] = (color.g * grad.g) / 255u;
                buffer[index + 2] = (color.b * grad.b) / 255u;
            }
        }
    }
}

static void draw_tetromino(uint8_t *buffer, tetromino_id_t id, int8_t base_x,
                           int8_t base_y, uint8_t rot, color_t color) {
    if (id == TETRIS_NONE)
        return;

    const tetromino_def_t *block = &TETRIS_BLOCKS[id];

    for (uint8_t y = 0; y < block->size; y++) {
        int8_t cell_y = base_y + y;
        if (cell_y < 0)
            continue;

        for (uint8_t x = 0; x < block->size; x++) {
            if (!get_rotated(block, rot, x, y))
                continue;

            int8_t cell_x = base_x + x;
            if (cell_x < 0)
                continue;

            draw_cell(buffer, cell_x, cell_y, color);
        }
    }
}

static void draw_box_border(uint8_t *buffer, uint8_t x0, uint8_t y0, uint8_t w,
                            uint8_t h, color_t color) {
    for (uint8_t y = 0; y < h; y++) {
        for (uint8_t x = 0; x < w; x++) {
            bool on_edge = (x == 0 || y == 0 || x == w - 1 || y == h - 1);
            if (on_edge) {
                draw_cell(buffer, x0 + x, y0 + y, color);
            }
        }
    }
}

static inline bool is_bag_empty(void) { return state.bag == 0; }

static inline void get_new_bag(void) { state.bag = TETRIS_FULL_BAG_MASK; }

static tetromino_id_t get_tetromino(void) {
    if (state.bag == 0)
        return TETRIS_NONE;

    uint8_t available = 0;
    for (uint8_t i = 0; i < TETRIS_BAG_SIZE; i++) {
        if (state.bag & (1u << i))
            available++;
    }

    uint8_t target = rand() % available;
    uint8_t curr = 0;

    for (uint8_t i = 0; i < TETRIS_BAG_SIZE; i++) {
        uint8_t bit = 1u << i;
        if (state.bag & bit) {
            if (curr == target) {
                state.bag &= ~bit;
                return i;
            }
            curr++;
        }
    }

    return TETRIS_NONE;
}

static void reset_tetromino_pos() {
    state.curr_pos.x =
        (TETRIS_ARENA_WIDTH - TETRIS_BLOCKS[state.active_tetro].size) / 2;
    state.curr_pos.y = 0;
    if (state.active_tetro == TETRIS_I_IDX)
        state.curr_pos.y--;
    state.curr_pos.r = 0;
    state.has_tetromino = true;
}

static void spawn_tetromino(void) {
    state.active_tetro = get_tetromino();
    reset_tetromino_pos();
}

static bool move_down(void) {
    tetro_pos_t temp = {.x = state.curr_pos.x,
                        .y = state.curr_pos.y + 1,
                        .r = state.curr_pos.r};

    if (is_valid_position(&temp)) {
        state.curr_pos = temp;
        return true;
    }

    return false;
}

static void move(bool right) {
    tetro_pos_t temp = {
        .x = state.curr_pos.x, .y = state.curr_pos.y, .r = state.curr_pos.r};

    if (right)
        temp.x++;
    else
        temp.x--;

    if (is_valid_position(&temp))
        state.curr_pos = temp;
    state.move_delay = TETRIS_MOVE_DELAY;
}

static void drop(void) {
    while (move_down())
        ;
    state.move_delay = 1;
}

static void rotate(bool clockwise) {
    state.move_delay = TETRIS_MOVE_DELAY;
    if (state.active_tetro == TETRIS_O_IDX)
        return;

    uint8_t step = clockwise ? 1u : 3u;
    uint8_t next_r = (state.curr_pos.r + step) & TETRIS_ROT_MASK;
    bool is_I = (state.active_tetro == TETRIS_I_IDX);

    const int8_t (*kicks)[2];
    if (clockwise) {
        kicks = is_I ? I_CW_KICKS[next_r] : JLSTZ_CW_KICKS[next_r];
    } else {
        kicks = is_I ? I_CCW_KICKS[next_r] : JLSTZ_CCW_KICKS[next_r];
    }

    for (uint8_t i = 0; i < TETRIS_KICKTEST_COUNT; i++) {
        tetro_pos_t candidate = {.x = state.curr_pos.x + kicks[i][0],
                                 .y = state.curr_pos.y + kicks[i][1],
                                 .r = next_r};

        if (is_valid_position(&candidate)) {
            state.curr_pos = candidate;
            return;
        }
    }
}

static void hold(void) {
    if (state.held_tetro == TETRIS_NONE) {
        state.held_tetro = state.active_tetro;
        if (is_bag_empty())
            get_new_bag();
        spawn_tetromino();
    } else {
        tetromino_id_t temp = state.held_tetro;
        state.held_tetro = state.active_tetro;
        state.active_tetro = temp;
        reset_tetromino_pos();
    }

    state.move_delay = TETRIS_MOVE_DELAY;
}

static void handle_input(const hil_button_state_t *btn_state) {
    if (state.prev_key == *btn_state)
        return;

    switch (*btn_state) {
    case HIL_BUTTON_START:
        rotate(false);
        break;
    case HIL_BUTTON_SELECT:
        rotate(true);
        break;
    case HIL_BUTTON_DPAD_LEFT:
    case HIL_BUTTON_LEFT:
        move(false);
        break;
    case HIL_BUTTON_DPAD_RIGHT:
    case HIL_BUTTON_RIGHT:
        move(true);
        break;
    case HIL_BUTTON_DPAD_UP:
    case HIL_BUTTON_UP:
        hold();
        break;
    case HIL_BUTTON_DPAD_DOWN:
    case HIL_BUTTON_DOWN:
        drop();
        break;
    }

    state.prev_key = *btn_state;
}

static void lock_tetromino(void) {
    const tetromino_def_t *block = &TETRIS_BLOCKS[state.active_tetro];

    for (uint8_t y = 0; y < block->size; y++) {
        for (uint8_t x = 0; x < block->size; x++) {
            uint8_t cell = get_rotated(block, state.curr_pos.r, x, y);
            int8_t arena_x = state.curr_pos.x + x;
            int8_t arena_y = state.curr_pos.y + y;

            if (cell && arena_y >= 0 && arena_y < TETRIS_ARENA_HEIGHT &&
                arena_x >= 0 && arena_x < TETRIS_ARENA_WIDTH) {
                state.arena[arena_y][arena_x] = state.active_tetro + 1;
            }
        }
    }

    state.has_tetromino = false;
}

static void clear_lines(void) {
    state.clearing_lines = false;
    memset(state.full_rows, 0, sizeof(state.full_rows));

    for (int8_t y = TETRIS_ARENA_HEIGHT - 1; y >= 0; y--) {
        bool full = true;
        for (uint8_t x = 0; x < TETRIS_ARENA_WIDTH; x++) {
            if (state.arena[y][x] == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            state.full_rows[y] = true;
            state.clearing_lines = true;
            state.clear_delay = TETRIS_CLEAR_DELAY;
        }
    }
}

static void drop_cleared(void) {
    for (uint8_t y = TETRIS_ARENA_HEIGHT - 1; y > 0; y--) {
        if (state.full_rows[y]) {
            for (uint8_t row = y; row > 0; row--) {
                memcpy(state.arena[row], state.arena[row - 1],
                       TETRIS_ARENA_WIDTH);
                state.full_rows[row] = state.full_rows[row - 1];
            }

            memset(state.arena[0], 0, TETRIS_ARENA_WIDTH);
            state.full_rows[0] = false;

            y++;
        }
    }

    state.clearing_lines = false;
    state.has_tetromino = false;
}

static bool handle_game_over(void) {
    if (!is_valid_position(&state.curr_pos)) {
        state.game_over = true;
        return true;
    }
    return false;
}

void tetris_init(void) {
    srand(hil_get_time());
    memset(&state, 0, sizeof(tetris_state_t));

    state.move_delay = TETRIS_MOVE_DELAY;
    state.game_over_delay = TETRIS_GAMEOVER_DELAY;
    state.held_tetro = TETRIS_NONE;
}

void tetris_step(hil_button_state_t *btn_state) {
    if (state.game_over) {
        if (state.game_over_delay % TETRIS_BLINK_DELAY == 0)
            state.is_blinking = !state.is_blinking;
        if (state.game_over_delay == 0)
            tetris_init();
        state.game_over_delay--;
        return;
    }

    if (state.clear_delay > 0) {
        if (state.clear_delay % TETRIS_BLINK_DELAY == 0)
            state.is_blinking = !state.is_blinking;
        state.clear_delay--;
        return;
    } else if (state.clearing_lines) {
        drop_cleared();
        state.is_blinking = true;
        return;
    }

    if (!state.has_tetromino) {
        if (is_bag_empty())
            get_new_bag();

        spawn_tetromino();

        if (handle_game_over())
            return;
    }

    handle_input(btn_state);

    if (--state.move_delay != 0)
        return;
    state.move_delay = TETRIS_MOVE_DELAY;

    if (!move_down()) {
        lock_tetromino();
        clear_lines();
        if (!state.clearing_lines)
            state.has_tetromino = false;
    }
}

void tetris_draw(void) {
    uint8_t *buffer = hil_get_display_buffer();
    memset(buffer, 0, SCREEN_SIZE_X * SCREEN_SIZE_Y * 3);

    // border
    draw_box_border(buffer, TETRIS_ARENA_X_OFFSET - 1,
                    TETRIS_ARENA_Y_OFFSET - 1, TETRIS_ARENA_WIDTH + 2,
                    TETRIS_ARENA_HEIGHT + 2, TETRIS_BORDER_COLOR);

    // hold area
    const uint8_t box_x = TETRIS_ARENA_X_OFFSET + TETRIS_ARENA_WIDTH + 2;
    const uint8_t box_y = TETRIS_ARENA_Y_OFFSET - 1;
    const uint8_t inner_size = TETRIS_ARENA_HELD_SIZE - 2;
    draw_box_border(buffer, box_x, box_y, TETRIS_ARENA_HELD_SIZE,
                    TETRIS_ARENA_HELD_SIZE, TETRIS_BORDER_COLOR);

    // held tetromino
    if (state.held_tetro != TETRIS_NONE) {
        const tetromino_def_t *block = &TETRIS_BLOCKS[state.held_tetro];
        uint8_t hold_x = (box_x + 1) + (inner_size - block->size) / 2;
        uint8_t hold_y = (box_y + 1) + (inner_size - block->size) / 2 + 1;

        if (state.held_tetro == TETRIS_I_IDX ||
            state.held_tetro == TETRIS_O_IDX)
            hold_y--;

        draw_tetromino(buffer, state.held_tetro, hold_x, hold_y, 0,
                       TETRIS_COLORS[state.held_tetro]);
    }

    // locked tetrominoes
    for (uint8_t y = 0; y < TETRIS_ARENA_HEIGHT; y++) {
        for (uint8_t x = 0; x < TETRIS_ARENA_WIDTH; x++) {
            if (state.arena[y][x] != 0) {
                color_t color = (state.full_rows[y] && state.is_blinking)
                                    ? TETRIS_BLACK_COLOR
                                    : TETRIS_COLORS[state.arena[y][x] - 1];
                draw_cell(buffer, x + TETRIS_ARENA_X_OFFSET,
                          y + TETRIS_ARENA_Y_OFFSET, color);
            }
        }
    }

    // active tetromino
    if ((!state.game_over && state.has_tetromino) ||
        (state.game_over && state.is_blinking)) {
        draw_tetromino(buffer, state.active_tetro,
                       state.curr_pos.x + TETRIS_ARENA_X_OFFSET,
                       state.curr_pos.y + TETRIS_ARENA_Y_OFFSET,
                       state.curr_pos.r, TETRIS_COLORS[state.active_tetro]);
    }
}