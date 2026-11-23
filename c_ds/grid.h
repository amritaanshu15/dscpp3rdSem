/*
 * grid.h  -  the forest map
 * DS topic: 2D array (multidimensional array), recursion (DFS flood fill)
 *
 * The map is a 2D char array tile[ROWS][COLS].
 * Address of tile[i][j] (row major) = base + (i * COLS + j) * sizeof(char)
 */
#ifndef GRID_H
#define GRID_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char tile[ROWS][COLS];
} Grid;

void grid_fill(Grid *g, char t);                    /* put same tile everywhere          */
int  grid_inside(int r, int c);                      /* 1 if (r,c) is on the map          */
char grid_get(const Grid *g, int r, int c);          /* outside the map = rock            */
void grid_set(Grid *g, int r, int c, char t);
int  grid_count(const Grid *g, char t);              /* how many tiles of this type       */
void grid_print(const Grid *g);                      /* print map in terminal (cli mode)  */

/* makes a random map. Rabbit starts at (startR, startC).
   Uses recursive DFS to check the map is playable, else makes a new one. */
void grid_generate(Grid *g, int startR, int startC);

/* recursive DFS: counts how many walkable cells can be reached from (r,c)
   and sets *waterNear = 1 if some reachable cell is next to water */
int  grid_reachable(const Grid *g, int r, int c, int *waterNear);

#ifdef __cplusplus
}
#endif

#endif
