#ifndef __MINESWEEPER_H
#define __MINESWEEPER_H

#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>

#define TILE_OPENED  0b00010000
#define TILE_MINED   0b00100000
#define TILE_FLAGGED 0b01000000
#define COUNT_MASK   0b00001111

#define TEXT_WIN      "YOU WON!"
#define TEXT_GAMEOVER "GAME OVER"
#define TEXT_EXIT     "Press 'q' to exit"

// east, north east, north, north west, west, south west, south, south east
const int dir_x[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int dir_y[] = {0, -1, -1, -1, 0, 1, 1, 1};

enum gamestatus {
    INPROGRESS,
    GAMEOVER,
    WIN,
    EXIT
};

typedef struct {
    int first;
    int second;
} pair_t;

typedef struct {
    enum gamestatus status;
    int mines;
    char minefield[9][9];
    pair_t size;
    pair_t cursor;
    pair_t * queue;
    char ** visited;
} gamedata_t;

gamedata_t * initgame();
void endgame(gamedata_t *);
void showgame(gamedata_t *);
void input(gamedata_t *);
void flood(gamedata_t *);
void checkflags(gamedata_t *);

#endif

