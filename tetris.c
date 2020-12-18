//======================================================================================================================
// File Name    : tetris.c
// Description  : Implementation of tetris using ncurses
// Authors      : Liam Lawrence
// Created      : December 17, 2020
// License      : MIT License
// Copyright    : (c) 2020, Liam Lawrence
// Todo         : Make hold
//======================================================================================================================

#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "tetris.h"

// MACROS //
#define BAG_SIZE            7
#define GRAV_LEVELS         15
#define PF_W                10
#define PF_H                40
#define TETROMINO_SPAWN_X   3
#define TETROMINO_SPAWN_Y   (PF_H-1-22) // examine why `update_playfield()` uses `PF_BUFF_SIZE` and not the y offset
// UI
#define PRINT_BLOCK         "\u2588"
#define X_SCALE             2
#define GUTTER_SPACE        (1*X_SCALE)
// PLAYFIELD UI
#define PF_PADDING          2
#define PF_BUFF_SIZE        19
#define PLAYFIELD_WIDTH     (PF_W*X_SCALE)
#define PLAYFIELD_HEIGHT    (PF_H-PF_BUFF_SIZE)
#define PLAYFIELD_X         2
#define PLAYFIELD_Y         1
// SCOREBOARD UI
#define SB_PADDING          2
#define SCOREBOARD_HEIGHT   9
#define SCOREBOARD_WIDTH    (9*X_SCALE)
#define SCOREBOARD_X        (PLAYFIELD_WIDTH+PF_PADDING+SB_PADDING+GUTTER_SPACE)
#define SCOREBOARD_Y        1
// NEXT PIECE UI
#define NP_PADDING          2
#define NEXTP_HEIGHT        9
#define NEXTP_WIDTH         (9*X_SCALE)
#define NEXTP_X             (PLAYFIELD_WIDTH+PF_PADDING+NP_PADDING+GUTTER_SPACE)
#define NEXTP_Y             (SCOREBOARD_HEIGHT+SCOREBOARD_Y+NP_PADDING)


// TYPEDEFS, PROTOTYPES, STRUCTS, & ENUMS //
enum directions_e {
    DIR_LRD = 0,
    DIR_CW,
    DIR_CCW,
};

typedef enum {
    I_tet = 1,
    O_tet,
    T_tet,
    S_tet,
    Z_tet,
    J_tet,
    L_tet,
} shapes_t;

enum colors_e {
    tI_c = I_tet,
    tO_c = O_tet,
    tT_c = T_tet,
    tS_c = S_tet,
    tZ_c = Z_tet,
    tJ_c = J_tet,
    tL_c = L_tet,
    bg_c,
    borders_c,
    buffer_c,
    buffI_c,
    buffO_c,
    buffT_c,
    buffS_c,
    buffZ_c,
    buffJ_c,
    buffL_c,
};

typedef struct {
    shapes_t shape;
    int x;
    int y;
    int rotation;
    uint16_t bitmap;
    bool falling;
} tetromino_t;

typedef struct {
    shapes_t tetrominos[BAG_SIZE];
    int idx;
} bag_t;

static void shuffle_bag(bag_t *B);
static void tetris_init(void);
static void tetris_close(void);
static void update_scoreboard(const int score, const int lines, const int level);
static void update_nextp(const shapes_t shape);
static void update_playfield(const uint8_t playfield[PF_H][PF_W], tetromino_t *tet);


// FUNCTIONS //---------------------------------------------------------------------------------------------------------
// update the bitmap of a tetromino based on its shape and rotation
static void update_tetromino(tetromino_t *tet)
{
    switch (tet->shape) {
        case I_tet:
            switch (tet->rotation) {
                case 0:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b1111 << 8u) |
                                  (0b0000 << 4u) |
                                  (0b0000);
                    break;

                case 1:
                    tet->bitmap = (0b0010 << 12u) |
                                  (0b0010 << 8u) |
                                  (0b0010 << 4u) |
                                  (0b0010);
                    break;

                case 2:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b0000 << 8u) |
                                  (0b1111 << 4u) |
                                  (0b0000);
                    break;

                case 3:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b0100 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0100);
                    break;
            }
            break;


        case O_tet:
            tet->bitmap = (0b0110 << 12u) |
                          (0b0110 << 8u) |
                          (0b0000 << 4u) |
                          (0b0000);
            break;


        case T_tet:
            switch (tet->rotation) {
                case 0:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b1110 << 8u) |
                                  (0b0000 << 4u) |
                                  (0b0000);
                    break;

                case 1:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b0110 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;

                case 2:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b1110 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;

                case 3:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b1100 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;
            }
            break;


        case S_tet:
            switch (tet->rotation) {
                case 0:
                    tet->bitmap = (0b0110 << 12u) |
                                  (0b1100 << 8u) |
                                  (0b0000 << 4u) |
                                  (0b0000);
                    break;

                case 1:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b0110 << 8u) |
                                  (0b0010 << 4u) |
                                  (0b0000);
                    break;

                case 2:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b0110 << 8u) |
                                  (0b1100 << 4u) |
                                  (0b0000);
                    break;

                case 3:
                    tet->bitmap = (0b1000 << 12u) |
                                  (0b1100 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;
            }
            break;


        case Z_tet:
            switch (tet->rotation) {
                case 0:
                    tet->bitmap = (0b1100 << 12u) |
                                  (0b0110 << 8u) |
                                  (0b0000 << 4u) |
                                  (0b0000);
                    break;

                case 1:
                    tet->bitmap = (0b0010 << 12u) |
                                  (0b0110 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;

                case 2:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b1100 << 8u) |
                                  (0b0110 << 4u) |
                                  (0b0000);
                    break;

                case 3:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b1100 << 8u) |
                                  (0b1000 << 4u) |
                                  (0b0000);
                    break;
            }
            break;


        case J_tet:
            switch (tet->rotation) {
                case 0:
                    tet->bitmap = (0b1000 << 12u) |
                                  (0b1110 << 8u) |
                                  (0b0000 << 4u) |
                                  (0b0000);
                    break;

                case 1:
                    tet->bitmap = (0b0110 << 12u) |
                                  (0b0100 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;

                case 2:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b1110 << 8u) |
                                  (0b0010 << 4u) |
                                  (0b0000);
                    break;

                case 3:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b0100 << 8u) |
                                  (0b1100 << 4u) |
                                  (0b0000);
                    break;
            }
            break;


        case L_tet:
            switch (tet->rotation) {
                case 0:
                    tet->bitmap = (0b0010 << 12u) |
                                  (0b1110 << 8u) |
                                  (0b0000 << 4u) |
                                  (0b0000);
                    break;

                case 1:
                    tet->bitmap = (0b0100 << 12u) |
                                  (0b0100 << 8u) |
                                  (0b0110 << 4u) |
                                  (0b0000);
                    break;

                case 2:
                    tet->bitmap = (0b0000 << 12u) |
                                  (0b1110 << 8u) |
                                  (0b1000 << 4u) |
                                  (0b0000);
                    break;

                case 3:
                    tet->bitmap = (0b1100 << 12u) |
                                  (0b0100 << 8u) |
                                  (0b0100 << 4u) |
                                  (0b0000);
                    break;
            }
            break;
    }
}

// draw a tetromino on a window
static void draw_tetromino(WINDOW *win, tetromino_t *tet, const int yoff, const int xoff)
{
    int x, y;
    uint16_t bm = tet->bitmap;

    wattron(win, COLOR_PAIR(tet->shape));
    for (int i = 0; i < 16; i++) {
        x = (i % 4) + tet->x;
        y = (i / 4) + tet->y;

        if ((bm >> (15-i)) & 1) {
            mvwprintw(win, y+yoff, (x*X_SCALE)+xoff, PRINT_BLOCK);
            mvwprintw(win, y+yoff, (x*X_SCALE)+xoff+1, PRINT_BLOCK);
        }
    }
    wattroff(win, COLOR_PAIR(tet->shape));
}

// returns 1 if there was a collision, otherwise returns 0 and updates the tetromino's coordinates
static int collision(tetromino_t *tet, const uint8_t playfield[PF_H][PF_W],
                     enum directions_e dir, const int yoff, const int xoff)
{
    enum {
        NO_COLLISION = 0,
        ERR_COLLISION,
    };

    int x, y;
    uint16_t bm = tet->bitmap;

    // Translation left, right, down
    if (dir == DIR_LRD) {
        for (int i = 0; i < 16; i++) {
            x = (i % 4) + tet->x + xoff;
            y = (i / 4) + tet->y + yoff;

            if ((bm >> (15 - i)) & 1) {
                if (x < 0 || x >= PF_W || y >= PF_H || playfield[y][x])
                    return ERR_COLLISION;
            }
        }
        tet->x += xoff;
        tet->y += yoff;
        return NO_COLLISION;
    }


    // Rotation clockwise, counter-clockwise
    if (tet->shape == O_tet)
        return NO_COLLISION;

    int table_idx;
    tetromino_t tmp;
    struct point_s {
        int x;
        int y;
    };
    // WARNING: Y values are inverted
    struct point_s JLSTZ_wallkick[8][5] = {{{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}},
                                           {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}},
                                           {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}},
                                           {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}},
                                           {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}},
                                           {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}},
                                           {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}},
                                           {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}}};
    struct point_s I_wallkick[8][5] =     {{{0, 0}, {-2, 0}, {+1, 0}, {+1, +2}, {-2, -1}},
                                           {{0, 0}, {+2, 0}, {-1, 0}, {+2, +1}, {-1, -2}},
                                           {{0, 0}, {-1, 0}, {+2, 0}, {-1, +2}, {+2, -1}},
                                           {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -1}},
                                           {{0, 0}, {+2, 0}, {-1, 0}, {+2, +1}, {-1, -1}},
                                           {{0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1}},
                                           {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2}},
                                           {{0, 0}, {+2, 0}, {-1, 0}, {-1, +2}, {+2, -1}}};
    switch (tet->rotation) {
        case 0:
            table_idx = (dir == DIR_CW) ? 0 : 7;
            break;
        case 1:
            table_idx = (dir == DIR_CW) ? 2 : 1;
            break;
        case 2:
            table_idx = (dir == DIR_CW) ? 4 : 3;
            break;
        case 3:
            table_idx = (dir == DIR_CW) ? 6 : 5;
            break;
    }
    tet->rotation = (dir == DIR_CW) ? (tet->rotation + 1) % 4 : (tet->rotation + 3) % 4;
    update_tetromino(tet);

    switch (tet->shape) {
        case I_tet:
            for (int i = 0; i < 5; i++) {
                tmp = *tet;
                if (collision(&tmp, playfield, DIR_LRD, -I_wallkick[table_idx][i].y, I_wallkick[table_idx][i].x))
                    continue;
                tet->x += I_wallkick[table_idx][i].x;
                tet->y -= I_wallkick[table_idx][i].y;
                return NO_COLLISION;
            }
            break;

        case T_tet:
        case S_tet:
        case Z_tet:
        case J_tet:
        case L_tet:
            for (int i = 0; i < 5; i++) {
                tmp = *tet;
                if (collision(&tmp, playfield, DIR_LRD, -JLSTZ_wallkick[table_idx][i].y, JLSTZ_wallkick[table_idx][i].x))
                    continue;
                tet->x += JLSTZ_wallkick[table_idx][i].x;
                tet->y -= JLSTZ_wallkick[table_idx][i].y;
                return NO_COLLISION;
            }
            break;
    }

    tet->rotation = (dir == DIR_CW) ? (tet->rotation + 3) % 4 : (tet->rotation + 1) % 4;
    update_tetromino(tet);
    return ERR_COLLISION;
}

// Copy a tetromino into the playfield once it has dropped
static void tet2playfield(tetromino_t *tet, uint8_t playfield[PF_H][PF_W])
{
    int x, y;
    uint16_t bm = tet->bitmap;

    for (int i = 0; i < 16; i++) {
        x = (i % 4) + tet->x;
        y = (i / 4) + tet->y;

        if (((bm >> (15-i)) & 1) && x < PF_W && y < PF_H && x >= 0 && y >= 0)
            playfield[y][x] = tet->shape;
    }
}

// play the game
static int tetris_run(void)
{
    bool running;
    uint8_t playfield[PF_H][PF_W] = {{0}};
    int ch;

    // Timing
    struct timespec fc_s, fc_e, gv_s, gv_e, sld_s, sld_e;
    double duration;
    double update_speed = 16667;
    const int gravity[GRAV_LEVELS] = {1000000, 793000, 617800, 472730, 355200, 262000, 189680,
                                      134730, 93880, 64150, 42980, 28220, 18150, 11440, 7060};   // (us / drop) / level
    // Scoring
    int score, lines, level;        // scoreboard stats
    int sum;                        // used to check if a line is full or not
    int lines_cleared = 0;          // used to count the numbers of lines cleared from a single drop, added to `lines`

    // Structs
    tetromino_t tetromino;
    shapes_t next_shape;
    bag_t bag;
    shuffle_bag(&bag);
    next_shape = bag.tetrominos[0];

    score = 0;
    lines = 0;
    level = 1;
    running = true;

    while (running) {
        // set up a new tetromino
        tetromino.shape = next_shape;
        if (++bag.idx == BAG_SIZE)
            shuffle_bag(&bag);
        next_shape = bag.tetrominos[bag.idx];
        tetromino.x = TETROMINO_SPAWN_X;
        tetromino.y = TETROMINO_SPAWN_Y;
        tetromino.rotation = 0;
        tetromino.falling = true;
        update_tetromino(&tetromino);

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fc_s);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &gv_s);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sld_s);
        while (tetromino.falling) {
            switch (ch = getch()) {
                // Left
                case 'a':
                    if (!collision(&tetromino, playfield, DIR_LRD, 0, -1))
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sld_s);
                    break;

                    // Right
                case 'd':
                    if (!collision(&tetromino, playfield, DIR_LRD, 0, 1))
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sld_s);
                    break;

                    // Down
                case 's':
                    if (collision(&tetromino, playfield, DIR_LRD, 1, 0))
                        tetromino.falling = false;
                    break;

                    // Clockwise
                case 'e':
                    if (!collision(&tetromino, playfield, DIR_CW, 0, 0))
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sld_s);
                    break;

                    // Counter-clockwise
                case 'q':
                    if(!collision(&tetromino, playfield, DIR_CCW, 0, 0))
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sld_s);
                    break;

                    // Hard drop
                case 'z':
                    while (!collision(&tetromino, playfield, DIR_LRD, 1, 0))
                        ;
                    tetromino.falling = false;
                    break;

                    // Quit
                case 'x':
                    tetromino.falling = false;
                    running = false;
                    break;
                case 'o':
                    level--;
                    break;
                case 'p':
                    level++;
                    break;
            }


            // screen UI refresh
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fc_e);
            duration = (double)(fc_e.tv_sec - fc_s.tv_sec) * 1e6 + (double)(fc_e.tv_nsec - fc_s.tv_nsec) / 1e3;
            if (duration >= update_speed) {
                update_playfield(playfield, &tetromino);
                update_scoreboard(score, lines, level);
                update_nextp(next_shape);
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fc_s);
            }

            // Gravity + 0.5s slide logic
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &gv_e);
            duration = (double)(gv_e.tv_sec - gv_s.tv_sec) * 1e6 + (double)(gv_e.tv_nsec - gv_s.tv_nsec) / 1e3;
            if (duration > gravity[level - 1]) {
                if (collision(&tetromino, playfield, DIR_LRD, 1, 0)) {
                    if (level != GRAV_LEVELS - 1) {
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sld_e);
                        duration = (double) (sld_e.tv_sec - sld_s.tv_sec) * 1e6 + (double) (sld_e.tv_nsec - sld_s.tv_nsec) / 1e3;
                        if (duration > 500000) {
                            tetromino.falling = false;
                            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &gv_s);
                        }
                    } else {
                        tetromino.falling = false;
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &gv_s);
                    }
                } else {
                    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &gv_s);
                }
            }
        }

        // Copy tetromino to the playfield buffer
        tet2playfield(&tetromino, playfield);

        // Check for game over
        for (int i = PF_BUFF_SIZE; i >= 0; i--) {
            for (int j = 0; j < PF_W; j++) {
                if (playfield[i][j]) {
                    running = false;
                }
            }
        }

        // Check for line clears
        for (int i = PF_H-1; i > PLAYFIELD_HEIGHT; i--) {
            sum = 0;
            for (int j = 0; j < PF_W; j++) {
                if (playfield[i][j])
                    sum++;
                else
                    break;
            }

            if (sum == PF_W) {
                for (int i2 = i; i2 > PLAYFIELD_HEIGHT; i2--) {
                    for (int j = 0; j < PF_W; j++) {
                        playfield[i2][j] = playfield[i2-1][j];
                    }
                }
                lines_cleared++;
                i++;
            }
        }

        // Increase score (and level) if there were line clears
        if (lines_cleared) {
            switch (lines_cleared) {
                case 1:
                    score += 100 * level;
                    break;
                case 2:
                    score += 300 * level;
                    break;
                case 3:
                    score += 500 * level;
                    break;
                case 4:
                    score += 800 * level;
                    break;
                default:
                    _exit(3);   // TODO: if this ever happens, add cases for more than 4 clears
            }
            lines += lines_cleared;
            level = (level == GRAV_LEVELS) ? GRAV_LEVELS : (lines / 10) + 1;
            lines_cleared = 0;
        }
    }

    // Game over
    const char *endstr = "\n             _____          __  __ ______  \n"
                         "            / ____|   /\\   |  \\/  |  ____|\n"
                         "           | |  __   /  \\  | \\  / | |__   \n"
                         "           | | |_ | / /\\ \\ | |\\/| |  __| \n"
                         "           | |__| |/ ____ \\| |  | | |____  \n"
                         "            \\_____/_/    \\_|_|  |_|______|\n\n"
                         "             ______      ________ _____  _ \n"
                         "            / __ \\ \\    / |  ____|  __ \\| |\n"
                         "           | |  | \\ \\  / /| |__  | |__) | |\n"
                         "           | |  | |\\ \\/ / |  __| |  _  /| |\n"
                         "           | |__| | \\  /  | |____| | \\ \\|_|\n"
                         "            \\____/   \\/   |______|_|  \\_(_)\n\n\n";
    attron(COLOR_PAIR(O_tet));
    mvprintw(5, 0, endstr);
    attroff(COLOR_PAIR(O_tet));
    refresh();
    getchar();
    return score;
}


// MAIN STRUCT //
tetris_t tetris = {.windows={NULL, NULL, NULL}, .init=&tetris_init, .run=&tetris_run, .close=&tetris_close};


// HELPER FUNCTIONS //
static void shuffle_bag(bag_t *B)
{
    int i, j;
    shapes_t tmp;

    // just in case a buffer overflows :)
    for (i = 0; i < BAG_SIZE; i++)
        B->tetrominos[i] = I_tet+i;

    for (i = BAG_SIZE-1; i > 0; i--) {
        j = rand() % (i+1);
        tmp = B->tetrominos[j];
        B->tetrominos[j] = B->tetrominos[i];
        B->tetrominos[i] = tmp;
    }

    B->idx = 0;
}

static void tetris_close(void)
{
    endwin();
}


// UPDATES //-----------------------------------------------------------------------------------------------------------
static void update_scoreboard(const int score, const int lines, const int level)
{
    werase(tetris.windows.scoreboard);
    wattron(tetris.windows.scoreboard, COLOR_PAIR(borders_c));
    box(tetris.windows.scoreboard, 0, 0);
    mvwprintw(tetris.windows.scoreboard, 1, 4, "SCORE  BOARD");
    mvwprintw(tetris.windows.scoreboard, 2, 0, "├──────────────────┤");
    mvwprintw(tetris.windows.scoreboard, 4, 2, "Score: %9d", score);
    mvwprintw(tetris.windows.scoreboard, 6, 2, "Lines: %9d", lines);
    mvwprintw(tetris.windows.scoreboard, 8, 2, "Level: %9d", level);
    wattroff(tetris.windows.scoreboard, COLOR_PAIR(borders_c));
    wrefresh(tetris.windows.scoreboard);
}

static void update_nextp(const shapes_t shape)
{
    int xoff, yoff;
    tetromino_t T = {.shape=shape, .rotation=0, .bitmap=0, .falling=false};

    switch (T.shape) {
        case I_tet:
        case O_tet:
            T.x = 3;
            T.y = 4;
            xoff = 0;
            yoff = 1;
            break;
        case T_tet:
        case S_tet:
        case Z_tet:
        case J_tet:
        case L_tet:
            T.x = 3;
            T.y = 4;
            xoff = 1;
            yoff = 1;
            break;

        default:
            _exit(1);
    }
    update_tetromino(&T);

    werase(tetris.windows.nextp);
    wattron(tetris.windows.nextp, COLOR_PAIR(borders_c));
    box(tetris.windows.nextp, 0, 0);
    mvwprintw(tetris.windows.nextp, 1, 5, "NEXT PIECE");
    mvwprintw(tetris.windows.nextp, 2, 0, "├──────────────────┤");
    wattroff(tetris.windows.nextp, COLOR_PAIR(borders_c));

    draw_tetromino(tetris.windows.nextp, &T, yoff, xoff);
    wrefresh(tetris.windows.nextp);
}

static void update_playfield(const uint8_t playfield[PF_H][PF_W], tetromino_t *tet)
{
    int x, y;
    int xoff = 1;
    int yoff = (-PF_BUFF_SIZE);

    // border
    werase(tetris.windows.playfield);
    wattron(tetris.windows.playfield, COLOR_PAIR(borders_c));
    box(tetris.windows.playfield, 0, 0);
    mvwprintw(tetris.windows.playfield, 0, 0, "│                    │");
    //for (int i = 0; i < PLAYFIELD_HEIGHT; i++)
    //    mvwprintw(tetris.windows.playfield,PLAYFIELD_HEIGHT-i-1, PF_W*X_SCALE-4, "%d", i+1);
    wattroff(tetris.windows.playfield, COLOR_PAIR(borders_c));

    // dropped pieces on the playfield
    for (int i = (-yoff); i < PF_H; i++) {
        for (int j = 0; j < PF_W; j++) {
            if (playfield[i][j]) {
                wattron(tetris.windows.playfield, COLOR_PAIR(playfield[i][j]));
                mvwprintw(tetris.windows.playfield, i+yoff, (j*X_SCALE)+xoff, PRINT_BLOCK);
                mvwprintw(tetris.windows.playfield, i+yoff, (j*X_SCALE)+xoff+1, PRINT_BLOCK);
                wattroff(tetris.windows.playfield, COLOR_PAIR(playfield[i][j]));
            }
        }
    }

    // current tetromino
    draw_tetromino(tetris.windows.playfield, tet, yoff, xoff);

    // if tetromino is peeking from behind the buffer
    wattron(tetris.windows.playfield, COLOR_PAIR(buffer_c));
    mvwprintw(tetris.windows.playfield, 0, 1, "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀");
    wattroff(tetris.windows.playfield, COLOR_PAIR(buffer_c));
    if (tet->y < PF_BUFF_SIZE+1) {
        wattron(tetris.windows.playfield, COLOR_PAIR(buffer_c+tet->shape));
        for (int i = 0; i < 16; i++) {
            x = (i % 4) + tet->x;
            y = (i / 4) + tet->y;

            if (y == PF_BUFF_SIZE && ((tet->bitmap >> (15 - i)) & 1)) {
                mvwprintw(tetris.windows.playfield, 0, (x*X_SCALE)+1, "▀");
                mvwprintw(tetris.windows.playfield, 0, (x*X_SCALE)+2, "▀");
            }
        }
        wattroff(tetris.windows.playfield, COLOR_PAIR(buffer_c+tet->shape));
    }

    wrefresh(tetris.windows.playfield);
}


// INIT //--------------------------------------------------------------------------------------------------------------
static void init_windows(void)
{
    tetris.windows.playfield = newwin(PLAYFIELD_HEIGHT+1, PLAYFIELD_WIDTH + PF_PADDING, PLAYFIELD_Y, PLAYFIELD_X);
    tetris.windows.scoreboard = newwin(SCOREBOARD_HEIGHT + SB_PADDING, SCOREBOARD_WIDTH + SB_PADDING, SCOREBOARD_Y, SCOREBOARD_X);
    tetris.windows.nextp = newwin(NEXTP_HEIGHT + NP_PADDING, NEXTP_WIDTH + NP_PADDING, NEXTP_Y, NEXTP_X);
}

static void init_colors(void)
{
    enum {
        background_col = COLOR_BLACK,
        playfield_border_col = COLOR_WHITE,
        buffer_col = COLOR_WHITE,
        tI_col = COLOR_CYAN,
        tO_col = COLOR_YELLOW,
        tT_col = COLOR_MAGENTA,    // purple
        tS_col = COLOR_GREEN,
        tZ_col = COLOR_RED,
        tJ_col = COLOR_BLUE,
        tL_col = COLOR_WHITE,      // orange
    };

    // main colors
    start_color();
    init_pair(bg_c, background_col, background_col);
    init_pair(borders_c, playfield_border_col, background_col);
    init_pair(buffer_c, buffer_col, background_col);

    // tetromino colors
    init_pair(tI_c, tI_col, background_col);
    init_pair(tO_c, tO_col, background_col);
    init_pair(tT_c, tT_col, background_col);
    init_pair(tS_c, tS_col, background_col);
    init_pair(tZ_c, tZ_col, background_col);
    init_pair(tJ_c, tJ_col, background_col);
    init_pair(tL_c, tL_col, background_col);

    // playfield buffer color when a tetromino is peeking
    init_pair(buffI_c, buffer_col, tI_col);
    init_pair(buffO_c, buffer_col, tO_col);
    init_pair(buffT_c, buffer_col, tT_col);
    init_pair(buffS_c, buffer_col, tS_col);
    init_pair(buffZ_c, buffer_col, tZ_col);
    init_pair(buffJ_c, buffer_col, tJ_col);
    init_pair(buffL_c, buffer_col, tL_col);
}

static void tetris_init(void)
{
    setlocale(LC_ALL, "");      // Enables unicode characters
    initscr();                  // Init ncurses
    init_colors();              // Init color pairs for ncurses
    init_windows();             // Init ncurses windows

    nodelay(stdscr, TRUE);      // getchr() will be non-blocking
    curs_set(0);                // cursor won't blink
    cbreak();                   // don't need to press enter to input a character
    noecho();                   // stdin won't be shown in the terminal
}
