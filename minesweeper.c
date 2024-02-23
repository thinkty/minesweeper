#include "minesweeper.h"

int main(void)
{
    setlocale(LC_ALL, "");

    // Initialize ncurses
    initscr();
    if (cbreak() != OK || noecho() != OK || keypad(stdscr, TRUE) != OK || curs_set(0) == ERR) {
        endwin();
        return ERR;
    }

    // Generate minefield
    gamedata_t * game = initgame();
    if (!game) {
        endwin();
        return ERR;
    }

    // Main game loop
    while (game->status != EXIT) {
        checkflags(game);
        showgame(game);
        input(game);
    }

    endgame(game);
    endwin();
    return OK;
}

gamedata_t * initgame()
{
    gamedata_t * game = calloc(1, sizeof(gamedata_t));
    if (!game) {
        return NULL;
    }

    game->status = INPROGRESS;
    game->mines = 10; // TODO: should probably change depending on level
    game->size.first = sizeof(game->minefield) / sizeof(game->minefield[0]);
    game->size.second = sizeof(game->minefield[0]) / sizeof(game->minefield[0][0]);

    game->queue = calloc(game->size.first * game->size.second, sizeof(pair_t));
    if (!game->queue) {
        free(game);
        return NULL;
    }

    game->visited = calloc(game->size.first, sizeof(pair_t *));
    if (!game->visited) {
        free(game->queue);
        free(game);
        return NULL;
    }
    for (int r = 0; r < game->size.first; r++) {
        game->visited[r] = calloc(game->size.second, sizeof(char));
        if (!game->visited[r]) {
            while (--r) {
                free(game->visited[r]);
            }
            free(game->visited);
            free(game->queue);
            free(game);
            return NULL;
        }
    }

    // Lay mines
    srandom((unsigned int) time(NULL));
    int i = game->mines;
    while (i) {
        int row = random() % game->size.first;
        int col = random() % game->size.second;

        if (game->minefield[row][col] & TILE_MINED) {
            continue;
        }
        game->minefield[row][col] |= TILE_MINED;
        i--;
    }

    // Put the count into tiles
    for (int r = 0; r < game->size.first; r++) {
        for (int c = 0; c < game->size.second; c++) {
            
            int mines = 0;
            
            for (int i = 0; i < 8; i++) {
                int row = r + dir_y[i];
                int col = c + dir_x[i];

                if (row < 0 || row >= game->size.first ||
                    col < 0 || col >= game->size.second) {
                    continue;
                }

                if (game->minefield[row][col] & TILE_MINED) {
                    mines++;
                }
            }

            game->minefield[r][c] |= (mines & COUNT_MASK);
        }
    }

    return game;
}

void endgame(gamedata_t * game)
{
    if (game) {
        if (game->queue) {
            free(game->queue);
        }
    
        if (game->visited) {
            for (int row = 0; row < game->size.first; row++) {
                free(game->visited[row]);
            }
            free(game->visited);
        }

        free(game);
    }
    return;
}

void showgame(gamedata_t * game)
{
    if (!game) {
        return;
    }

    erase();

    int y_offset = (getmaxy(stdscr) - game->size.first) / 2;
    int x_offset = (getmaxx(stdscr) - game->size.second * 2) / 2;

    for (int row = 0; row < game->size.first; row++) {
        for (int col = 0; col < game->size.second; col++) {

            char tile = game->minefield[row][col];
            wchar_t displayed;

            if (tile & TILE_OPENED) {
                if (tile & TILE_MINED) {
                    displayed = 0x2316;
                } else if (tile & COUNT_MASK) {
                    // For tiles with counts, display the digit
                    displayed = 0x0030 + (tile & COUNT_MASK);
                } else {
                    // For tiles with no counts, display space
                    displayed = 0x0020;
                }
            } else if (tile & TILE_FLAGGED) {
                displayed = 0x2690;
            } else {
                // Not opened nor flagged, display empty tile
                displayed = 0x2b24;
            }

            unsigned long int attr = A_NORMAL;
            if (row == game->cursor.first && col == game->cursor.second) {
                attr |= A_STANDOUT;
            }

            attron(attr);
            mvprintw(y_offset + row, x_offset + col*2, "%lc", displayed);
            attroff(attr);
        }
    }


    switch (game->status) {
        case GAMEOVER:
            y_offset = (getmaxy(stdscr) + game->size.first) / 2 + 1;
            x_offset = (getmaxx(stdscr) - strlen(TEXT_GAMEOVER)) / 2;
            mvprintw(y_offset, x_offset, TEXT_GAMEOVER);
            y_offset++;
            x_offset = (getmaxx(stdscr) - strlen(TEXT_EXIT)) / 2;
            mvprintw(y_offset, x_offset, TEXT_EXIT);
            break;

        case WIN:
            y_offset = (getmaxy(stdscr) + game->size.first) / 2 + 1;
            x_offset = (getmaxx(stdscr) - strlen(TEXT_WIN)) / 2;
            mvprintw(y_offset, x_offset, TEXT_WIN);
            y_offset++;
            x_offset = (getmaxx(stdscr) - strlen(TEXT_EXIT)) / 2;
            mvprintw(y_offset, x_offset, TEXT_EXIT);
            break;

        case INPROGRESS:
        case EXIT:
        default:
            break;
    }

    refresh();
    return;
}

void input(gamedata_t * game)
{
    int row = game->cursor.first, col = game->cursor.second;
    
    // If the game has ended, press q to exit
    if (game->status != INPROGRESS) {
        if (getch() == 'q') {
            game->status = EXIT;
        }
        return;
    }

    switch (getch()) {
        case 'q': // quit game
            game->status = EXIT;
            break;

        case 'h': // move left
            if (game->cursor.second > 0) {
                game->cursor.second--;
            }
            break;

        case 'l': // move right
            if (game->cursor.second < game->size.second-1) {
                game->cursor.second++;
            }
            break;

        case 'k': // move up
            if (game->cursor.first > 0) {
                game->cursor.first--;
            }
            break;

        case 'j': // move down
            if (game->cursor.first < game->size.first-1) {
                game->cursor.first++;
            }
            break;

        case 'd': // dig
            // Can only dig unopened tiles
            if (game->minefield[row][col] & TILE_OPENED) {
                break;
            }

            // Open current tile and surrounding 0 tiles
            flood(game);

            // If the tile was mined, gameover 
            if (game->minefield[row][col] & TILE_MINED) {
                game->status = GAMEOVER;
            }
            break;

        case 'f': // flag        
            // Can only flag unopened tiles
            if (game->minefield[row][col] & TILE_OPENED) {
                break;
            }

            // Toggle flag
            game->minefield[row][col] ^= TILE_FLAGGED;
            break;

        default: 
            break;
    }

    return;
}

void flood(gamedata_t * game) 
{
    for (int row = 0; row < game->size.first; row++) {
        for (int col = 0; col < game->size.second; col++) {
            game->visited[row][col] = 0;
        }
    }

    int qhead = 0, qtail = 0;
    game->queue[qtail++] = game->cursor;
    game->visited[game->cursor.first][game->cursor.second] = 1;

    while (qhead < qtail) {
        pair_t curr = game->queue[qhead++];
        game->minefield[curr.first][curr.second] |= TILE_OPENED;

        // If it's a number tile or a mined tile, do not add any neighboring tiles
        if (game->minefield[curr.first][curr.second] & (COUNT_MASK | TILE_MINED)) {
            continue;
        }

        for (int i = 0; i < 8; i++) {
            int row = curr.first + dir_y[i];
            int col = curr.second + dir_x[i];

            if (row < 0 || row >= game->size.first || col < 0 || 
                col >= game->size.second) {
                continue;
            }

            // Check that it has not been visited yet
            if (game->visited[row][col]) {
                continue;
            }

            game->visited[row][col] = 1;
            game->queue[qtail].first = row;
            game->queue[qtail].second = col;
            qtail++;
        }
    }

    return;
}

void checkflags(gamedata_t * game)
{
    int flagged = 0;

    // If all the tiles have been opened or flagged, check for correctness
    for (int row = 0; row < game->size.first; row++) {
        for (int col = 0; col < game->size.second; col++) {
            
            char tile = game->minefield[row][col];

            // If there exists a tile that has not been opened nor flagged,
            // game is still in progress.
            if ((tile & (TILE_OPENED | TILE_FLAGGED)) == 0) {
                return;
            }

            if (tile & TILE_FLAGGED) {
                flagged++;
            }
        }
    }

    game->status = flagged == game->mines ? WIN : GAMEOVER;
    return;
}

