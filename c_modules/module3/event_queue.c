#include <stdio.h>
#include "event_queue.h"


void queue_init(EventQueue *q)
{
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

int queue_enqueue(EventQueue *q, EventRec e)
{
    if (q->count == MAX_EVENTS) return 0;
    q->rear = (q->rear + 1) % MAX_EVENTS;      
    q->items[q->rear] = e;
    q->count++;
    return 1;
}

int queue_dequeue(EventQueue *q, EventRec *out)
{
    if (q->count == 0) return 0;
    *out = q->items[q->front];
    q->front = (q->front + 1) % MAX_EVENTS;
    q->count--;
    return 1;
}

int queue_is_empty(const EventQueue *q)
{
    return q->count == 0;
}

int queue_list(const EventQueue *q, EventRec out[])
{
    int i;
    for (i = 0; i < q->count; i++)
        out[i] = q->items[(q->front + i) % MAX_EVENTS];
    return q->count;
}

void queue_display(const EventQueue *q)
{
    int i;
    printf("FRONT -> ");
    for (i = 0; i < q->count; i++) {
        const EventRec *e = &q->items[(q->front + i) % MAX_EVENTS];
        printf("[#%d %s] ", e->id, e->name);
    }
    printf("<- REAR\n");
}

static int comes_first(const EventRec *a, const EventRec *b)
{
    if (a->priority != b->priority) return a->priority > b->priority;
    return a->id < b->id;
}

static void swap_events(EventRec *a, EventRec *b)
{
    EventRec t = *a; *a = *b; *b = t;
}

void pq_init(EventPQ *p)
{
    p->count = 0;
}

int pq_push(EventPQ *p, EventRec e)
{
    int i;
    if (p->count == MAX_EVENTS) return 0;
    i = p->count++;
    p->items[i] = e;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (comes_first(&p->items[i], &p->items[parent])) {
            swap_events(&p->items[i], &p->items[parent]);
            i = parent;
        } else break;
    }
    return 1;
}

int pq_pop(EventPQ *p, EventRec *out)
{
    int i = 0;
    if (p->count == 0) return 0;
    *out = p->items[0];
    p->items[0] = p->items[--p->count];
    for (;;) {
        int left = 2 * i + 1, right = 2 * i + 2, best = i;
        if (left  < p->count && comes_first(&p->items[left],  &p->items[best])) best = left;
        if (right < p->count && comes_first(&p->items[right], &p->items[best])) best = right;
        if (best == i) break;
        swap_events(&p->items[i], &p->items[best]);
        i = best;
    }
    return 1;
}

int pq_is_empty(const EventPQ *p)
{
    return p->count == 0;
}

int pq_list(const EventPQ *p, EventRec out[])
{
    EventPQ copy = *p;
    int n = 0;
    while (pq_pop(&copy, &out[n])) n++;
    return n;
}

void pq_display(const EventPQ *p)
{
    EventRec tmp[MAX_EVENTS];
    int i, n = pq_list(p, tmp);
    printf("NEXT -> ");
    for (i = 0; i < n; i++)
        printf("[#%d %s prio=%d] ", tmp[i].id, tmp[i].name, tmp[i].priority);
    printf("\n");
}


void history_init(EventHistory *h)
{
    h->count = 0;
}

void history_add(EventHistory *h, EventRec e, int day, const char *effect)
{
    HistoryRec *r;
    int i;
    if (h->count == MAX_HISTORY) {                 
        for (i = 1; i < MAX_HISTORY; i++) h->items[i - 1] = h->items[i];
        h->count--;
    }
    r = &h->items[h->count++];
    r->ev = e;
    r->processedDay = day;
    strncpy(r->effect, effect, EFFECT_LEN - 1);
    r->effect[EFFECT_LEN - 1] = '\0';
}
