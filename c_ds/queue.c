/*
 * queue.c  -  circular queue
 * rear and front move forward with  (index + 1) % QSIZE  so the array is
 * used again from the start (that is why it is called circular)
 */
#include "queue.h"

void q_init(Queue *q)
{
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

int q_isEmpty(const Queue *q)
{
    return q->count == 0;
}

int q_isFull(const Queue *q)
{
    return q->count == QSIZE;
}

int q_enqueue(Queue *q, QItem item)
{
    if (q_isFull(q))
        return 0;                          /* overflow */
    q->items[q->rear] = item;
    q->rear = (q->rear + 1) % QSIZE;
    q->count++;
    return 1;
}

int q_dequeue(Queue *q, QItem *out)
{
    if (q_isEmpty(q))
        return 0;                          /* underflow */
    *out = q->items[q->front];
    q->front = (q->front + 1) % QSIZE;
    q->count--;
    return 1;
}

int q_peek(const Queue *q, QItem *out)
{
    if (q_isEmpty(q))
        return 0;
    *out = q->items[q->front];
    return 1;
}
