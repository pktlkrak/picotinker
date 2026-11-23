#include "tetris.h"
#include "tetris_data.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static uint8_t arena[TETRIS_ARENA_HEIGHT][TETRIS_ARENA_WIDTH] = {0};
static uint8_t bag[TETRIS_BAG_SIZE] = {0};
static uint8_t delay = TETRIS_MOVE_DELAY;

static tet_pos_t curr_pos;
static tetromino_id_t current;
static tetromino_id_t held = TETRIS_NONE;
static bool has_tetromino = false;
static hil_button_state_t prev_key = 0;

static uint8_t clear_delay = 0;
static uint8_t game_over_delay = TETRIS_GAMEOVER_DELAY;
static bool clearing_lines = false;
static bool blink = true;
static bool game_over = false;
static bool full_rows[TETRIS_ARENA_HEIGHT] = {false};

uint8_t get_rotated(const tetromino_def_t *block, uint8_t rot, uint8_t x,
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
        break;
    }

    return block->cells[src_y * block->size + src_x];
}

bool is_valid_position(tetromino_id_t *curr, tet_pos_t *pos) {
    const tetromino_def_t *block = &TETRIS_BLOCKS[*curr];

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

            if (arena[arena_y][arena_x] != 0) {
                return false;
            }
        }
    }

    return true;
}

void draw_cell(uint8_t *buffer, uint8_t x, uint8_t y, color_t color) {
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
                buffer[index + 0] = (color.r * grad.r) / 255;
                buffer[index + 1] = (color.g * grad.g) / 255;
                buffer[index + 2] = (color.b * grad.b) / 255;
            }
        }
    }
}

bool is_bag_empty() {
    static const uint8_t zeros[TETRIS_BAG_SIZE] = {0};
    return memcmp(bag, zeros, TETRIS_BAG_SIZE) == 0;
}

void get_new_bag() { memset(bag, 1, TETRIS_BAG_SIZE); }

tetromino_id_t get_tetromino() {
    uint8_t available = 0;

    for (int i = 0; i < TETRIS_BAG_SIZE; i++) {
        if (bag[i] == 1)
            available++;
    }

    if (available == 0)
        return TETRIS_NONE;

    uint8_t target = rand() % available;
    uint8_t curr = 0;

    for (uint8_t i = 0; i < TETRIS_BAG_SIZE; i++) {
        if (bag[i] == 1) {
            if (curr == target) {
                bag[i] = 0;
                return i;
            }
            curr++;
        }
    }

    return TETRIS_NONE;
}

void spawn_tetromino(tetromino_id_t *curr, tet_pos_t *pos) {
    *curr = get_tetromino();
    pos->x = (TETRIS_ARENA_WIDTH - TETRIS_BLOCKS[*curr].size) / 2;
    pos->y = 0;
    if (*curr == TETRIS_I_IDX)
        pos->y--;
    pos->r = 0;
    has_tetromino = true;
}

bool move_down(tet_pos_t *pos) {
    tet_pos_t temp = {.x = pos->x, .y = pos->y + 1, .r = pos->r};

    if (is_valid_position(&current, &temp)) {
        *pos = temp;
        return true;
    }

    return false;
}

void move_left(tet_pos_t *pos) {
    tet_pos_t temp = {.x = pos->x - 1, .y = pos->y, .r = pos->r};

    if (is_valid_position(&current, &temp))
        *pos = temp;
    delay = TETRIS_MOVE_DELAY;
}

void move_right(tet_pos_t *pos) {
    tet_pos_t temp = {.x = pos->x + 1, .y = pos->y, .r = pos->r};

    if (is_valid_position(&current, &temp))
        *pos = temp;
    delay = TETRIS_MOVE_DELAY;
}

void drop(tet_pos_t *pos) {
    while (move_down(pos))
        ;
    delay = 1;
}

void rotate_clockwise(tet_pos_t *pos) {
    delay = TETRIS_MOVE_DELAY;
    if (current == TETRIS_O_IDX)
        return;

    uint8_t next_r = (pos->r + 1) & 3;

    tet_pos_t temp = {.x = pos->x, .y = pos->y, .r = next_r};

    const int8_t (*kicks)[2] =
        (current == TETRIS_I_IDX) ? I_CW_KICKS[next_r] : JLSTZ_CW_KICKS[next_r];

    for (uint8_t i = 0; i < TETRIS_KICKTEST_COUNT; i++) {
        temp.x = pos->x + kicks[i][0];
        temp.y = pos->y + kicks[i][1];

        if (is_valid_position(&current, &temp)) {
            *pos = temp;
            return;
        }
    }
}

void rotate_anticlockwise(tet_pos_t *pos) {
    delay = TETRIS_MOVE_DELAY;
    if (current == TETRIS_O_IDX)
        return;

    uint8_t next_r = (pos->r + 3) & 3;

    tet_pos_t temp = {.x = pos->x, .y = pos->y, .r = next_r};

    const int8_t (*kicks)[2] = (current == TETRIS_I_IDX)
                                   ? I_CCW_KICKS[next_r]
                                   : JLSTZ_CCW_KICKS[next_r];

    for (uint8_t i = 0; i < TETRIS_KICKTEST_COUNT; i++) {
        temp.x = pos->x + kicks[i][0];
        temp.y = pos->y + kicks[i][1];

        if (is_valid_position(&current, &temp)) {
            *pos = temp;
            return;
        }
    }
}

void hold(tet_pos_t *pos) {
    if (held == TETRIS_NONE) {
        held = current;
        if (is_bag_empty())
            get_new_bag();
        spawn_tetromino(&current, &curr_pos);
    } else {
        tetromino_id_t temp = held;
        held = current;
        current = temp;

        pos->x = (TETRIS_ARENA_WIDTH - TETRIS_BLOCKS[current].size) / 2;
        pos->y = 0;
        if (current == TETRIS_I_IDX)
            pos->y--;
        pos->r = 0;
        has_tetromino = true;
    }

    delay = TETRIS_MOVE_DELAY;
}

void handle_input(hil_button_state_t *btn_state) {
    if (prev_key == *btn_state)
        return;

    switch (*btn_state) {
    case HIL_BUTTON_START:
        rotate_anticlockwise(&curr_pos);
        break;
    case HIL_BUTTON_SELECT:
        rotate_clockwise(&curr_pos);
        break;
    case HIL_BUTTON_DPAD_LEFT:
    case HIL_BUTTON_LEFT:
        move_left(&curr_pos);
        break;
    case HIL_BUTTON_DPAD_RIGHT:
    case HIL_BUTTON_RIGHT:
        move_right(&curr_pos);
        break;
    case HIL_BUTTON_DPAD_UP:
    case HIL_BUTTON_UP:
        hold(&curr_pos);
        break;
    case HIL_BUTTON_DPAD_DOWN:
    case HIL_BUTTON_DOWN:
        drop(&curr_pos);
        break;
    }

    prev_key = *btn_state;
}

void lock_tetromino(tetromino_id_t *curr, tet_pos_t *pos) {
    const tetromino_def_t *block = &TETRIS_BLOCKS[*curr];

    for (uint8_t y = 0; y < block->size; y++) {
        for (uint8_t x = 0; x < block->size; x++) {
            uint8_t cell = get_rotated(block, pos->r, x, y);
            int8_t arena_x = pos->x + x;
            int8_t arena_y = pos->y + y;

            if (cell && arena_y >= 0 && arena_y < TETRIS_ARENA_HEIGHT &&
                arena_x >= 0 && arena_x < TETRIS_ARENA_WIDTH) {
                arena[arena_y][arena_x] = *curr + 1;
            }
        }
    }

    has_tetromino = false;
}

void clear_lines() {
    clearing_lines = false;
    memset(full_rows, 0, sizeof(full_rows));

    for (int8_t y = TETRIS_ARENA_HEIGHT - 1; y >= 0; y--) {
        bool full = true;
        for (uint8_t x = 0; x < TETRIS_ARENA_WIDTH; x++) {
            if (arena[y][x] == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            full_rows[y] = true;
            clearing_lines = true;
            clear_delay = TETRIS_CLEAR_DELAY;
        }
    }
}

void drop_cleared(void) {
    for (uint8_t y = TETRIS_ARENA_HEIGHT - 1; y > 0; y--) {
        if (full_rows[y]) {
            for (uint8_t row = y; row > 0; row--) {
                memcpy(arena[row], arena[row - 1], TETRIS_ARENA_WIDTH);
                full_rows[row] = full_rows[row - 1];
            }

            memset(arena[0], 0, TETRIS_ARENA_WIDTH);
            full_rows[0] = false;

            y++;
        }
    }

    clearing_lines = false;
    has_tetromino = false;
}

bool handle_game_over() {
    if (!is_valid_position(&current, &curr_pos)) {
        game_over = true;
        return true;
    }
    return false;
}

void reset(void) {
    memset(arena, 0, sizeof(arena));
    memset(bag, 0, sizeof(bag));
    delay = TETRIS_MOVE_DELAY;

    held = TETRIS_NONE;
    has_tetromino = false;
    prev_key = 0;

    clear_delay = 0;
    game_over_delay = TETRIS_GAMEOVER_DELAY;
    clearing_lines = false;
    blink = true;
    game_over = false;
    memset(full_rows, 0, sizeof(full_rows));
}

void step(hil_button_state_t *btn_state) {
    if (game_over) {
        if (game_over_delay % TETRIS_BLINK_DELAY == 0)
            blink = !blink;
        if (game_over_delay == 0)
            reset();
        game_over_delay--;
        return;
    }

    if (clear_delay > 0) {
        if (clear_delay % TETRIS_BLINK_DELAY == 0)
            blink = !blink;
        clear_delay--;
        return;
    } else if (clearing_lines) {
        drop_cleared();
        blink = true;
        return;
    }

    if (!has_tetromino) {
        if (is_bag_empty())
            get_new_bag();

        spawn_tetromino(&current, &curr_pos);

        if (handle_game_over())
            return;
    }

    handle_input(btn_state);

    if (--delay != 0)
        return;
    delay = TETRIS_MOVE_DELAY;

    if (!move_down(&curr_pos)) {
        lock_tetromino(&current, &curr_pos);
        clear_lines();
        if (!clearing_lines)
            has_tetromino = false;
    }
}

void draw() {
    uint8_t *buffer = hil_get_display_buffer();
    memset(buffer, 0, SCREEN_SIZE_X * SCREEN_SIZE_Y * 3);

    // border
    for (uint8_t y = 0; y < TETRIS_ARENA_HEIGHT + 2; y++) {
        for (uint8_t x = 0; x < TETRIS_ARENA_WIDTH + 2; x++) {
            if (y != TETRIS_ARENA_HEIGHT + 1 && y != 0 && x != 0 &&
                x != TETRIS_ARENA_WIDTH + 1)
                continue;
            draw_cell(buffer, x + TETRIS_ARENA_X_OFFSET - 1,
                      y + TETRIS_ARENA_Y_OFFSET - 1, TETRIS_BORDER_COLOR);
        }
    }

    // hold area
    const uint8_t box_x = TETRIS_ARENA_X_OFFSET + TETRIS_ARENA_WIDTH + 2;
    const uint8_t box_y = TETRIS_ARENA_Y_OFFSET;
    const uint8_t inner_size = TETRIS_ARENA_HELD_SIZE - 2;

    for (uint8_t y = 0; y < TETRIS_ARENA_HELD_SIZE; y++) {
        for (uint8_t x = 0; x < TETRIS_ARENA_HELD_SIZE; x++) {
            if (y != 0 && x != 0 && y != TETRIS_ARENA_HELD_SIZE - 1 &&
                x != TETRIS_ARENA_HELD_SIZE - 1)
                continue;
            draw_cell(buffer, box_x + x, box_y + y, TETRIS_BORDER_COLOR);
        }
    }

    // held tetromino
    if (held != TETRIS_NONE) {
        const tetromino_def_t *block = &TETRIS_BLOCKS[held];

        int8_t offset_x =
            (int8_t)(box_x + 1) + (int8_t)(inner_size - block->size) / 2;
        int8_t offset_y =
            (int8_t)(box_y + 1) + (int8_t)(inner_size - block->size) / 2 + 1;

        if (held == TETRIS_I_IDX || held == TETRIS_O_IDX)
            offset_y--;

        for (uint8_t y = 0; y < block->size; y++) {
            for (uint8_t x = 0; x < block->size; x++) {
                uint8_t cell = get_rotated(block, 0, x, y);
                if (cell) {
                    uint8_t arena_x = (uint8_t)(offset_x + (int8_t)x);
                    uint8_t arena_y = (uint8_t)(offset_y + (int8_t)y);
                    draw_cell(buffer, arena_x, arena_y, TETRIS_COLORS[held]);
                }
            }
        }
    }

    // locked tetrominoes
    for (uint8_t y = 0; y < TETRIS_ARENA_HEIGHT; y++) {
        for (uint8_t x = 0; x < TETRIS_ARENA_WIDTH; x++) {
            if (arena[y][x] != 0) {
                color_t color;
                if (full_rows[y] && !blink)
                    color = TETRIS_BLACK_COLOR;
                else
                    color = TETRIS_COLORS[arena[y][x] - 1];
                draw_cell(buffer, x + TETRIS_ARENA_X_OFFSET,
                          y + TETRIS_ARENA_Y_OFFSET, color);
            }
        }
    }

    // active tetromino
    if ((!game_over && has_tetromino) || (game_over && !blink)) {
        const tetromino_def_t *block = &TETRIS_BLOCKS[current];
        for (uint8_t y = 0; y < block->size; y++) {
            for (uint8_t x = 0; x < block->size; x++) {
                uint8_t cell = get_rotated(block, curr_pos.r, x, y);
                if (cell) {
                    uint8_t arena_x = curr_pos.x + x;
                    int8_t arena_y = curr_pos.y + y;
                    if (arena_y < 0)
                        continue;
                    draw_cell(buffer, arena_x + TETRIS_ARENA_X_OFFSET,
                              arena_y + TETRIS_ARENA_Y_OFFSET,
                              TETRIS_COLORS[current]);
                }
            }
        }
    }
}