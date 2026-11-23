/*
 * grid.c  -  forest map functions
 */
#include <stdio.h>
#include <stdlib.h>
#include "grid.h"

void grid_fill(Grid *g, char t)
{
    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            g->tile[i][j] = t;
}

int grid_inside(int r, int c)
{
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        return 1;
    return 0;
}

char grid_get(const Grid *g, int r, int c)
{
    if (!grid_inside(r, c))
        return T_ROCK;           /* outside of map works like a wall */
    return g->tile[r][c];
}

void grid_set(Grid *g, int r, int c, char t)
{
    if (grid_inside(r, c))
        g->tile[r][c] = t;
}

int grid_count(const Grid *g, char t)
{
    int i, j, count = 0;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            if (g->tile[i][j] == t)
                count++;
    return count;
}

void grid_print(const Grid *g)
{
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++)
            printf("%c ", g->tile[i][j]);
        printf("\n");
    }
}

/* ---------------- recursive DFS (flood fill) ---------------- */

static int walkable(char t)
{
    /* rock and water stop the rabbit */
    if (t == T_ROCK || t == T_WATER)
        return 0;
    return 1;
}

static int dr[4] = {-1, 1, 0, 0};    /* up, down, left, right */
static int dc[4] = {0, 0, -1, 1};

static int dfs(const Grid *g, int r, int c, int visited[ROWS][COLS], int *waterNear)
{
    int k, count;

    if (!grid_inside(r, c)) return 0;            /* base case 1: outside      */
    if (visited[r][c]) return 0;                 /* base case 2: already seen */
    if (!walkable(g->tile[r][c])) {
        if (g->tile[r][c] == T_WATER)
            *waterNear = 1;                      /* we touched a pond */
        return 0;
    }

    visited[r][c] = 1;
    count = 1;
    for (k = 0; k < 4; k++)
        count = count + dfs(g, r + dr[k], c + dc[k], visited, waterNear);   /* recursive call */
    return count;
}

int grid_reachable(const Grid *g, int r, int c, int *waterNear)
{
    int visited[ROWS][COLS];
    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            visited[i][j] = 0;

    *waterNear = 0;
    return dfs(g, r, c, visited, waterNear);
}

/* ---------------- random map ---------------- */

static void put_pond(Grid *g)
{
    /* a pond is a small plus shape of water */
    int r = 1 + rand() % (ROWS - 2);
    int c = 1 + rand() % (COLS - 2);
    grid_set(g, r, c, T_WATER);
    grid_set(g, r + 1, c, T_WATER);
    grid_set(g, r, c + 1, T_WATER);
    if (rand() % 2 == 0) grid_set(g, r - 1, c, T_WATER);
    if (rand() % 2 == 0) grid_set(g, r, c - 1, T_WATER);
}

void grid_generate(Grid *g, int startR, int startC)
{
    int tries, i, j, k, x;
    int reach, waterNear, free;

    for (tries = 0; tries < 50; tries++) {
        grid_fill(g, T_GROUND);

        /* random grass and rocks */
        for (i = 0; i < ROWS; i++) {
            for (j = 0; j < COLS; j++) {
                x = rand() % 100;
                if (x < 30)      g->tile[i][j] = T_GRASS;   /* 30% grass */
                else if (x < 38) g->tile[i][j] = T_ROCK;    /*  8% rock  */
            }
        }

        /* 3 ponds */
        for (k = 0; k < 3; k++)
            put_pond(g);

        /* 7 bushes for hiding */
        for (k = 0; k < 7; k++)
            grid_set(g, rand() % ROWS, rand() % COLS, T_BUSH);

        /* start cell and its 4 neighbours must be free */
        grid_set(g, startR, startC, T_GROUND);
        grid_set(g, startR - 1, startC, T_GROUND);
        grid_set(g, startR + 1, startC, T_GROUND);
        grid_set(g, startR, startC - 1, T_GROUND);
        grid_set(g, startR, startC + 1, T_GROUND);

        /* check: rabbit can reach most of the map AND some water */
        free = ROWS * COLS - grid_count(g, T_ROCK) - grid_count(g, T_WATER);
        reach = grid_reachable(g, startR, startC, &waterNear);

        if (waterNear == 1 && reach * 100 >= free * 85)
            return;          /* good map */
    }
    /* after 50 bad maps we just keep the last one (almost never happens) */
}
