/*
 * pathfind.h  -  BFS shortest path on the map
 * DS topic: Graph traversal (BFS), queue, stack
 *
 * We think of the map as a GRAPH:
 *   every walkable cell = a node
 *   two cells next to each other (up/down/left/right) = an edge
 * BFS from the fox gives the shortest path to the rabbit.
 */
#ifndef PATHFIND_H
#define PATHFIND_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * canWalk[r][c] = 1 if the animal can stand on that cell, else 0
 * start  = where the animal is
 * target = where it wants to go (target is always allowed)
 * next   = OUT: the first step it should take
 * returns number of steps of the shortest path, or -1 if there is no path
 */
int bfs_next_step(int canWalk[ROWS][COLS], Cell start, Cell target, Cell *next);

#ifdef __cplusplus
}
#endif

#endif
