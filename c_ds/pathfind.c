/*
 * pathfind.c  -  BFS using our circular queue, path rebuilt using our stack
 */
#include "pathfind.h"
#include "queue.h"
#include "stack.h"

int bfs_next_step(int canWalk[ROWS][COLS], Cell start, Cell target, Cell *next)
{
    int visited[ROWS][COLS];
    Cell parent[ROWS][COLS];          /* parent[r][c] = cell we came from */
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};
    int i, j, k, found = 0;
    Queue q;
    QItem item, now;
    Stack s;
    Cell cur, step;

    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            visited[i][j] = 0;

    if (start.r == target.r && start.c == target.c) {
        *next = start;
        return 0;
    }

    /* ---- BFS ---- */
    q_init(&q);
    item.r = start.r; item.c = start.c; item.day = 0; item.tile = 0;
    q_enqueue(&q, item);
    visited[start.r][start.c] = 1;

    while (!q_isEmpty(&q) && !found) {
        q_dequeue(&q, &now);

        for (k = 0; k < 4; k++) {
            int nr = now.r + dr[k];
            int nc = now.c + dc[k];

            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (visited[nr][nc]) continue;

            int isTarget = (nr == target.r && nc == target.c);
            if (!canWalk[nr][nc] && !isTarget) continue;

            visited[nr][nc] = 1;
            parent[nr][nc].r = now.r;
            parent[nr][nc].c = now.c;

            if (isTarget) { found = 1; break; }

            item.r = nr; item.c = nc;
            q_enqueue(&q, item);
        }
    }

    if (!found)
        return -1;                        /* rabbit can not be reached */

    /* ---- walk back from target to start, pushing every cell on the stack ---- */
    st_init(&s);
    cur = target;
    while (!(cur.r == start.r && cur.c == start.c)) {
        st_push(&s, cur);
        cur = parent[cur.r][cur.c];
    }

    /* the top of the stack is the cell right next to the start */
    k = st_size(&s);                      /* length of the path */
    st_pop(&s, &step);
    *next = step;
    return k;
}
