/*
 * queue.h  -  circular queue (array)
 * DS topic: Queue, circular queue, operations: create, add, delete, full, empty
 *
 * Used for 2 things in the game:
 *   1. BFS in pathfind.c (fox finding the rabbit)
 *   2. grass regrow list in World (eaten grass comes back after some days)
 */
#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QSIZE 200        /* map has 192 cells, so 200 is enough */

typedef struct {
    int  r;
    int  c;
    int  day;            /* regrow: the day when it grows back (BFS does not use it) */
    char tile;           /* regrow: what tile comes back (grass or bush)            */
} QItem;

typedef struct {
    QItem items[QSIZE];
    int front;           /* index of first item          */
    int rear;            /* index where next item goes   */
    int count;           /* how many items are inside    */
} Queue;

void q_init(Queue *q);
int  q_isEmpty(const Queue *q);
int  q_isFull(const Queue *q);
int  q_enqueue(Queue *q, QItem item);       /* 1 = ok, 0 = overflow  */
int  q_dequeue(Queue *q, QItem *out);       /* 1 = ok, 0 = underflow */
int  q_peek(const Queue *q, QItem *out);    /* look at front without removing */

#ifdef __cplusplus
}
#endif

#endif
