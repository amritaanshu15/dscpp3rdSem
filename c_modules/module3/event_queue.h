#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "../common/eco_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_EVENTS   50
#define MAX_HISTORY 100
#define EFFECT_LEN  160

#define EVT_DROUGHT          1
#define EVT_FOREST_FIRE      2
#define EVT_PREDATOR_REMOVAL 3
#define EVT_HEAVY_RAIN       4
#define EVT_SEASONAL_CHANGE  5

typedef struct {
    int  id;
    int  type;
    int  priority;
    int  day;
    char name[NAME_LEN];
} EventRec;

typedef struct {
    EventRec items[MAX_EVENTS];
    int front, rear, count;
} EventQueue;

void queue_init(EventQueue *q);
int  queue_enqueue(EventQueue *q, EventRec e);
int  queue_dequeue(EventQueue *q, EventRec *out);
int  queue_is_empty(const EventQueue *q);
int  queue_list(const EventQueue *q, EventRec out[]);
void queue_display(const EventQueue *q);

typedef struct {
    EventRec items[MAX_EVENTS];
    int count;
} EventPQ;

void pq_init(EventPQ *p);
int  pq_push(EventPQ *p, EventRec e);
int  pq_pop(EventPQ *p, EventRec *out);
int  pq_is_empty(const EventPQ *p);
int  pq_list(const EventPQ *p, EventRec out[]);
void pq_display(const EventPQ *p);

typedef struct {
    EventRec ev;
    int      processedDay;
    char     effect[EFFECT_LEN];
} HistoryRec;

typedef struct {
    HistoryRec items[MAX_HISTORY];
    int count;
} EventHistory;

void history_init(EventHistory *h);
void history_add(EventHistory *h, EventRec e, int day, const char *effect);

#ifdef __cplusplus
}
#endif
#endif
