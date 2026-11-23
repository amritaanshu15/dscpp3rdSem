/*
 * event_heap.h  -  priority queue of future events (min heap)
 * DS topic: Priority queue, heap (array representation of a complete binary tree)
 *
 * At the start of the game we plan some events (drought, fire, rain ...)
 * on random days. The heap always gives back the event with the
 * SMALLEST day first, so the events come in the correct order.
 *
 * For index i:  parent = (i-1)/2   left child = 2i+1   right child = 2i+2
 */
#ifndef EVENT_HEAP_H
#define EVENT_HEAP_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_EVENTS 30

/* event type numbers (C++ Event classes use the same numbers) */
#define EV_DROUGHT     1
#define EV_FOREST_FIRE 2
#define EV_HEAVY_RAIN  3
#define EV_NEW_FOX     4
#define EV_HUNTER      5

typedef struct {
    int day;       /* when it happens (this is the priority, smaller = sooner) */
    int type;      /* EV_... */
} EventItem;

typedef struct {
    EventItem items[MAX_EVENTS];
    int count;
} EventHeap;

void heap_init(EventHeap *h);
int  heap_isEmpty(const EventHeap *h);
int  heap_push(EventHeap *h, EventItem e);     /* 1 = ok, 0 = full  */
int  heap_pop(EventHeap *h, EventItem *out);   /* removes the soonest event */
int  heap_peek(const EventHeap *h, EventItem *out);

#ifdef __cplusplus
}
#endif

#endif
