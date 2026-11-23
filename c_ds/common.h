/*
 * common.h
 * Things that every C file needs: size of the map and the tile letters.
 * (C++ files also include this, so the numbers are same everywhere)
 */
#ifndef COMMON_H
#define COMMON_H

#define ROWS 12          /* map height */
#define COLS 16          /* map width  */

/* one letter for every kind of tile on the map */
#define T_GROUND '.'     /* plain mud, anyone can walk          */
#define T_GRASS  'g'     /* rabbit and deer eat this            */
#define T_WATER  'w'     /* pond: nobody walks, rabbit drinks   */
#define T_BUSH   'b'     /* berry bush: rabbit hides here       */
#define T_ROCK   'r'     /* blocks walking (eagle flies over)   */
#define T_BURNT  'x'     /* burnt by forest fire                */

/* one cell of the map (row, column) */
typedef struct {
    int r;
    int c;
} Cell;

#endif
