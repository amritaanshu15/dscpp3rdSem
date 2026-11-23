/*
 * event_heap.c  -  min heap
 */
#include "event_heap.h"

static void swap(EventItem *a, EventItem *b)
{
    EventItem t = *a;
    *a = *b;
    *b = t;
}

void heap_init(EventHeap *h)
{
    h->count = 0;
}

int heap_isEmpty(const EventHeap *h)
{
    return h->count == 0;
}

int heap_push(EventHeap *h, EventItem e)
{
    int i, parent;
    if (h->count == MAX_EVENTS)
        return 0;

    /* put at the end, then move it UP while it is smaller than its parent */
    i = h->count;
    h->items[i] = e;
    h->count++;

    while (i > 0) {
        parent = (i - 1) / 2;
        if (h->items[i].day < h->items[parent].day) {
            swap(&h->items[i], &h->items[parent]);
            i = parent;
        } else {
            break;
        }
    }
    return 1;
}

int heap_pop(EventHeap *h, EventItem *out)
{
    int i, left, right, smallest;
    if (h->count == 0)
        return 0;

    *out = h->items[0];                       /* root = smallest day */
    h->count--;
    h->items[0] = h->items[h->count];         /* last item goes to the root */

    /* move it DOWN while a child is smaller */
    i = 0;
    while (1) {
        left = 2 * i + 1;
        right = 2 * i + 2;
        smallest = i;
        if (left < h->count && h->items[left].day < h->items[smallest].day)
            smallest = left;
        if (right < h->count && h->items[right].day < h->items[smallest].day)
            smallest = right;
        if (smallest == i)
            break;
        swap(&h->items[i], &h->items[smallest]);
        i = smallest;
    }
    return 1;
}

int heap_peek(const EventHeap *h, EventItem *out)
{
    if (h->count == 0)
        return 0;
    *out = h->items[0];
    return 1;
}
