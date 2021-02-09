#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "moves.h"
#include "board.h"

/*
 * Adds a potential move to the tracker inside the board struct.
 */
void add_move(struct board *board, int location, int type, int previous_location) {
    board->move_locations[board->move_location_tracker].move = type;
    board->move_locations[board->move_location_tracker].previous_location = previous_location;
    board->move_locations[board->move_location_tracker].location = location;
    board->move_location_tracker++;
}

/*
 * Returns an array containing array indices around a given x,y coordinate.
 */
void get_points_around(int y, int x, int* points) {
    points[0] = (y - 1) * BOARD_SIZE + (x + 0);
    points[1] = (y - 1) * BOARD_SIZE + (x - 1);
    points[2] = (y + 0) * BOARD_SIZE + (x - 1);
    points[3] = (y + 0) * BOARD_SIZE + (x + 1);
    points[4] = (y + 1) * BOARD_SIZE + (x + 0);
    points[5] = (y + 1) * BOARD_SIZE + (x + 1);
}

/*
 * Generates the placing moves, only allowed to place next to allied tiles.
 */
void generate_placing_moves(struct board *board, int type) {
    int color = type & COLOR_MASK;

//    int initial_position = (BOARD_SIZE / 2) * BOARD_SIZE + BOARD_SIZE / 2;
    int initial_position = (BOARD_SIZE * 2) + 2;
    if (board->turn == 0) {
        return add_move(board, initial_position, type, -1);
    }
    if (board->turn == 1) {
        return add_move(board, initial_position + 1, type, -1);
    }

    int* points = malloc(6 * sizeof(int));
    int* neighbour_points = malloc(6 * sizeof(int));
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            // Skip empty tiles
            if (board->tiles[y * BOARD_SIZE + x].type == NONE) continue;

            // Get points around this point
            get_points_around(y, x, points);
            for (int p = 0; p < 6; p++) {
                int index = points[p];
                // Check if its empty
                if (board->tiles[index].type != NONE) continue;

                // Check for all neighbours of this point if its the same colour as the colour of the
                //  passed tile.
                int invalid = 0;
                int yy = index / BOARD_SIZE;
                int xx = index % BOARD_SIZE;
                get_points_around(yy, xx, neighbour_points);
                for (int np = 0; np < 6; np++) {
                    int np_index = neighbour_points[np];
                    if (np_index < 0 || np_index >= BOARD_SIZE * BOARD_SIZE) continue;

                    if (board->tiles[np_index].type == NONE) continue;

                    // Check if every tile around it has the same colour as the passed tile colour.
                    if ((board->tiles[np_index].type & COLOR_MASK) != color) {
                        invalid = 1;
                        break;
                    }
                }

                // If any neighbour of this point is another colour, check another point.
                if (invalid) continue;

                // Check if this move is already tracked.
                int tracked = 0;
                for (int n = 0; n < board->move_location_tracker; n++) {
                    if (board->move_locations[n].location == index) {
                        tracked = 1;
                        break;
                    }
                }
                if (!tracked) {
                    add_move(board, index, type, -1);
                }
            }
        }
    }
    free(neighbour_points);
    free(points);
}

int add_if_unique(int *array, int n, int value) {
    for (int i = 0; i < n; i++) {
        if (array[i] == value) return 0;
    }
    array[n] = value;
    return 1;
}


int count_connected(struct board *board, int index) {
    int *connected = calloc(BOARD_SIZE * BOARD_SIZE, sizeof(int));
    int n_connected = 0;
    int *frontier = calloc(BOARD_SIZE * BOARD_SIZE, sizeof(int));
    int frontier_p = 0; // Frontier pointer.

    connected[n_connected++] = index;
    frontier[frontier_p++] = index;

    int* points = malloc(6 * sizeof(int));
    while (frontier_p != 0) {
        int i = frontier[--frontier_p];
        int y = i / BOARD_SIZE;
        int x = i % BOARD_SIZE;

        get_points_around(y, x, points);
        for (int n = 0; n < 6; n++) {
            // Skip this tile if theres nothing on it
            if (board->tiles[points[n]].type == NONE) continue;

            // If a unique tile is added, increment the tracker.
            int added = add_if_unique(connected, n_connected, points[n]);
            n_connected += added;

            if (added) {
                frontier[frontier_p++] = points[n];
            }
        }
    }
    free(points);
    free(frontier);
    free(connected);
    return n_connected;
}

int sum_hive_tiles(struct board *board) {
    return N_TILES * 2 - (
            board->player1.queens_left +
            board->player1.grasshoppers_left +
            board->player1.beetles_left +
            board->player1.ants_left +
            board->player1.spiders_left +
            board->player2.spiders_left +
            board->player2.ants_left +
            board->player2.queens_left +
            board->player2.grasshoppers_left +
            board->player2.beetles_left) -
            board->n_stacked;
}

void generate_directional_grasshopper_moves(struct board* board, int orig_y, int orig_x, int x_incr, int y_incr) {
    int orig_pos = orig_y * BOARD_SIZE + orig_x;
    int tile_type = board->tiles[orig_pos].type;

    int x = orig_x + x_incr;
    int y = orig_y + y_incr;
    // It needs to jump at least 1 tile.
    if (board->tiles[y * BOARD_SIZE + x].type != NONE) {
        while (1) {
            x += x_incr; y += y_incr;
            if (board->tiles[y * BOARD_SIZE + x].type == NONE) {
                add_move(board, y * BOARD_SIZE + x, tile_type, orig_pos);
                break;
            }
        }
    }

}

void generate_grasshopper_moves(struct board* board, int orig_y, int orig_x) {
    // Grasshopper can jump in the 6 directions over all connected sets of tiles
    generate_directional_grasshopper_moves(board, orig_y, orig_x,  0, -1);
    generate_directional_grasshopper_moves(board, orig_y, orig_x, -1, -1);
    generate_directional_grasshopper_moves(board, orig_y, orig_x,  1,  0);
    generate_directional_grasshopper_moves(board, orig_y, orig_x, -1,  0);
    generate_directional_grasshopper_moves(board, orig_y, orig_x,  0,  1);
    generate_directional_grasshopper_moves(board, orig_y, orig_x,  1,  1);
}

void generate_beetle_moves(struct board* board, int y, int x) {
    // Store tile type and simulate removal.
    int tile_type = board->tiles[y * BOARD_SIZE + x].type;
    board->tiles[y * BOARD_SIZE + x].type = NONE;


    int* points = malloc(6 * sizeof(int));
    int* neighbours = malloc(6 * sizeof(int));
    get_points_around(y, x, points);
    for (int p = 0; p < 6; p++) {
        int point = points[p];

        // Tile type doesnt matter in this case, because it can move on top of other tiles.

        int xx = point % BOARD_SIZE;
        int yy = point / BOARD_SIZE;

        // Check if placing the beetle on this position is valid (not creating 2 hives).
        int is_connected = 0;
        get_points_around(yy, xx, neighbours);
        for (int n = 0; n < 6; n++) {
            int neighbour = neighbours[n];

            if (board->tiles[neighbour].type != NONE) {
                is_connected = 1;
                break;
            }
        }

        if (is_connected) {
            add_move(board, point, tile_type, y * BOARD_SIZE + x);
        }
    }
    free(neighbours);
    free(points);

    board->tiles[y * BOARD_SIZE + x].type = tile_type;
}

bool has_neighbour(struct board* board, int location) {
    int x = location % BOARD_SIZE;
    int y = location / BOARD_SIZE;
    int* points = malloc(6 * sizeof(int));
    get_points_around(y, x, points);
    for (int i = 0; i < 6 ; i++) {
        if (points[i] < 0 || points[i] >= BOARD_SIZE * BOARD_SIZE) continue;

        if (board->tiles[points[i]].type != NONE) {
            free(points);
            return true;
        }
    }
    free(points);
    return false;
}

bool tile_fits(struct board* board, int x, int y, int new_x, int new_y) {
    bool tl = board->tiles[(y - 1) * BOARD_SIZE + (x - 1)].type == NONE;
    bool tr = board->tiles[(y - 1) * BOARD_SIZE + (x + 0)].type == NONE;
    bool l  = board->tiles[(y + 0) * BOARD_SIZE + (x - 1)].type == NONE;
    bool r  = board->tiles[(y + 0) * BOARD_SIZE + (x + 1)].type == NONE;
    bool bl = board->tiles[(y + 1) * BOARD_SIZE + (x + 0)].type == NONE;
    bool br = board->tiles[(y + 1) * BOARD_SIZE + (x + 1)].type == NONE;

    if (x - new_x == -1 && y - new_y == 0) {
        // Going to the right
        return tr || br;
    } else if (x - new_x == 1 && y - new_y == 0) {
        // Going to the left
        return tl || bl;
    } else if (x - new_x == 1 && y - new_y == 1) {
        // Going to the top left
        return l || tr;
    } else if (x - new_x == 0 && y - new_y == 1) {
        // Going to the top right
        return r || tl;
    } else if (x - new_x == 0 && y - new_y == -1) {
        // Going to the bottom left
        return l || br;
    } else if (x - new_x == -1 && y - new_y == -1) {
        // Going to the bottom right
        return r || bl;
    }
    printf("%d, %d\n", x - new_x, y - new_y);
    return false;
}

void generate_ant_moves(struct board* board, int orig_y, int orig_x) {
    // Store tile for temporary removal
    int tile_type = board->tiles[orig_y * BOARD_SIZE + orig_x].type;
    board->tiles[orig_y * BOARD_SIZE + orig_x].type = NONE;

    int *ant_move_buffer = calloc(BOARD_SIZE * BOARD_SIZE, sizeof(int));
    int n_ant_moves = 0;
    int *frontier = calloc(BOARD_SIZE * BOARD_SIZE, sizeof(int));
    int frontier_p = 0; // Frontier pointer.
    int index = orig_y * BOARD_SIZE + orig_x;

    ant_move_buffer[n_ant_moves++] = index;
    frontier[frontier_p++] = index;

    int* points = malloc(6 * sizeof(int));
    while (frontier_p != 0) {
        int i = frontier[--frontier_p];
        int y = i / BOARD_SIZE;
        int x = i % BOARD_SIZE;

        get_points_around(y, x, points);
        for (int n = 0; n < 6; n++) {
            int point = points[n];
            if (point < 0 || point >= BOARD_SIZE * BOARD_SIZE) continue;

            // Ants cannot stack.
            if (board->tiles[point].type != NONE) continue;

            // Skip this tile if it has no neighbours
            // Connected hive requirement.
            if (!has_neighbour(board, point)) continue;

            int new_x = point % BOARD_SIZE;
            int new_y = point / BOARD_SIZE;
            if (!tile_fits(board, x, y, new_x, new_y)) continue;

            // If a unique tile is added, increment the tracker.
            int added = add_if_unique(ant_move_buffer, n_ant_moves, points[n]);

            if (added) {
                n_ant_moves++;
                frontier[frontier_p++] = points[n];
            }
        }
    }
    free(points);
    board->tiles[orig_y * BOARD_SIZE + orig_x].type = tile_type;

    // Generate moves based on these valid ant moves.
    for (int i = 0; i < n_ant_moves; i++) {
        // Moving to the same position it already was is not valid.
        if (ant_move_buffer[i] == orig_y * BOARD_SIZE + orig_x) continue;
        add_move(board, ant_move_buffer[i], tile_type, orig_y * BOARD_SIZE + orig_x);
    }
    free(ant_move_buffer);
    free(frontier);
}

void generate_queen_moves(struct board* board, int y, int x) {
    // Store tile for temporary removal
    int tile_type = board->tiles[y * BOARD_SIZE + x].type;
    board->tiles[y * BOARD_SIZE + x].type = NONE;

    int* points = malloc(6 * sizeof(int));
    get_points_around(y, x, points);
    for (int i = 0; i < 6; i++) {
        int point = points[i];

        // Queens cannot stack
        if (board->tiles[point].type != NONE) continue;

        // Skip this tile if it has no neighbours
        // Connected hive requirement.
        if (!has_neighbour(board, point)) continue;

        int new_y = point / BOARD_SIZE;
        int new_x = point % BOARD_SIZE;
        if (!tile_fits(board, x, y, new_x, new_y)) continue;

        add_move(board, point, tile_type, y * BOARD_SIZE + x);
    }

    free(points);
    board->tiles[y * BOARD_SIZE + x].type = tile_type;
}

void generate_spider_moves(struct board* board, int orig_y, int orig_x) {
    // Store tile for temporary removal
    int tile_type = board->tiles[orig_y * BOARD_SIZE + orig_x].type;
    board->tiles[orig_y * BOARD_SIZE + orig_x].type = NONE;

    int* valid_moves = calloc(sizeof(int), BOARD_SIZE * BOARD_SIZE);
    int valid_moves_tracker = 0;

    // Frontier to track multiple points.
    int *frontier = calloc(BOARD_SIZE * BOARD_SIZE, sizeof(int));
    int frontier_p = 0; // Frontier pointer.
    int *next_frontier = calloc(BOARD_SIZE * BOARD_SIZE, sizeof(int));
    int next_frontier_p = 0; // Next frontier pointer.

    frontier[frontier_p++] = orig_y * BOARD_SIZE + orig_x;

    int* points = malloc(6 * sizeof(int));
    int* neighbour_points = malloc(6 * sizeof(int));

    for (int iteration = 0; iteration < 3; iteration++) {
        while (frontier_p > 0) {
            // Get a point on the frontier
            int frontier_point = frontier[--frontier_p];
            int x = frontier_point % BOARD_SIZE;
            int y = frontier_point / BOARD_SIZE;
            get_points_around(y, x, points);

            // Iterate all points surrounding this frontier.
            // Generate a list containing all valid moves from the frontier onward.
            for (int p = 0; p < 6; p++) {
                int point = points[p];
                if (board->tiles[point].type == NONE) continue;
                int xx = point % BOARD_SIZE;
                int yy = point / BOARD_SIZE;

                if (!tile_fits(board, x, y, xx, yy)) continue;

                // For all neighbours, if the neighbours neighbours == my neighbours -> valid move.
                get_points_around(yy, xx, neighbour_points);
                for (int i = 0; i < 6; i++) {
                    if (board->tiles[neighbour_points[i]].type != NONE) continue;
                    for (int j = 0; j < 6; j++) {
                        if (neighbour_points[i] == points[j]) {
                            if (!tile_fits(board, x, y, points[j] % BOARD_SIZE, points[j] / BOARD_SIZE)) continue;

                            // If a unique tile is added, increment the tracker.
                            int added = add_if_unique(valid_moves, valid_moves_tracker, neighbour_points[i]);
                            if (added) {
                                valid_moves_tracker++;
                                next_frontier[next_frontier_p++] = neighbour_points[i];
                            }
                        }
                    }
                }
            }
        }
        // Swap two frontiers.
        int* temp = frontier;
        frontier = next_frontier;
        next_frontier = temp;
        frontier_p = next_frontier_p;
        next_frontier_p = 0;
    }
    free(neighbour_points);
    free(points);
    board->tiles[orig_y * BOARD_SIZE + orig_x].type = tile_type;

    for (int i = 0; i < frontier_p; i++) {
        add_move(board, frontier[i], tile_type, orig_y * BOARD_SIZE + orig_x);
    }
    free(valid_moves);
    free(frontier);
    free(next_frontier);
}


void generate_free_moves(struct board *board, int player_bit) {
#ifdef DEBUG
    struct timespec start, end, start2, end2;
    double g_time, b_time, q_time, s_time, a_time;
    g_time = b_time = q_time = s_time = a_time = 0;
    unsigned int g_n, b_n, q_n, s_n, a_n;
    g_n = b_n = q_n = s_n = a_n = 0;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
#endif

    int n_hive_tiles = sum_hive_tiles(board) - 1;
    int* points = malloc(6 * sizeof(int));
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            struct tile *tile = &board->tiles[y * BOARD_SIZE + x];
            if (tile->type == NONE) continue;
            // Only move your own tiles
            if ((tile->type & COLOR_MASK) != player_bit) continue;

            // Store the tile type for later use
            int tile_type = tile->type;
            tile->type = NONE;

            // Check if the points around this point consist of a single spanning tree.
            int valid = 1;
            get_points_around(y, x, points);
            for (int p = 1; p < 6; p++) {
                int index = points[p];

                if (board->tiles[index].type == NONE) continue;

                // Count how many tiles are connected.
                int connect_count = count_connected(board, index);
                if (connect_count != n_hive_tiles) {
                    valid = 0;
                }
                break;
            }

            // Restore original tile value.
            tile->type = tile_type;

            if (valid) {
#ifdef DEBUG
                clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start2);
#endif
                // If this tile can be removed without breaking the hive, add it to the valid moves list.
                if ((tile->type & TILE_MASK) == L_GRASSHOPPER) {
                    generate_grasshopper_moves(board, y, x);
#ifdef DEBUG
                    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end2);
                    g_time += to_usec(end2) - to_usec(start2);
                    g_n++;
#endif
                } else if ((tile->type & TILE_MASK) == L_BEETLE) {
                    generate_beetle_moves(board, y, x);
#ifdef DEBUG
                    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end2);
                    b_time += to_usec(end2) - to_usec(start2);
                    b_n++;
#endif
                } else if ((tile->type & TILE_MASK) == L_ANT) {
                    generate_ant_moves(board, y, x);
#ifdef DEBUG
                    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end2);
                    a_time += to_usec(end2) - to_usec(start2);
                    a_n++;
#endif
                } else if ((tile->type & TILE_MASK) == L_QUEEN) {
                    generate_queen_moves(board, y, x);
#ifdef DEBUG
                    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end2);
                    q_time += to_usec(end2) - to_usec(start2);
                    q_n++;
#endif
                } else if ((tile->type & TILE_MASK) == L_SPIDER) {
                    generate_spider_moves(board, y, x);
#ifdef DEBUG
                    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end2);
                    s_time += to_usec(end2) - to_usec(start2);
                    s_n++;
#endif
                }
            }
        }
    }

    free(points);
#ifdef DEBUG
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
    double tot_time = to_usec(end) - to_usec(start);
    printf("----------------------------------\n");
    printf("Total us: %.5f\n", tot_time);
    printf("\tAvg. Spider us: %.5f (%d)\n", s_time/s_n, s_n);
    printf("\tAvg. Ant us: %.5f (%d)\n", a_time/a_n, a_n);
    printf("\tAvg. Grasshopper us: %.5f (%d)\n", g_time/g_n, g_n);
    printf("\tAvg. Beetle us: %.5f (%d)\n", b_time/b_n, b_n);
    printf("\tAvg. Queen us: %.5f (%d)\n", q_time/q_n, q_n);
#endif
}
