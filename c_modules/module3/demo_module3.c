#include <stdio.h>
#include "event_queue.h"

static EventRec make(int id, int type, int prio, const char *name)
{
    EventRec e;
    e.id = id; e.type = type; e.priority = prio; e.day = 0;
    eco_copy_name(e.name, name);
    return e;
}

int main(void)
{
    EventQueue q;
    EventPQ pq;
    EventRec e;

    queue_init(&q);
    pq_init(&pq);

    queue_enqueue(&q, make(1, EVT_SEASONAL_CHANGE, 1, "Seasonal Change"));
    queue_enqueue(&q, make(2, EVT_HEAVY_RAIN,      2, "Heavy Rain"));
    queue_enqueue(&q, make(3, EVT_FOREST_FIRE,     5, "Forest Fire"));
    pq_push(&pq, make(1, EVT_SEASONAL_CHANGE, 1, "Seasonal Change"));
    pq_push(&pq, make(2, EVT_HEAVY_RAIN,      2, "Heavy Rain"));
    pq_push(&pq, make(3, EVT_FOREST_FIRE,     5, "Forest Fire"));

    queue_display(&q);
    pq_display(&pq);

    printf("\nFIFO order    : ");
    while (queue_dequeue(&q, &e)) printf("%s, ", e.name);
    printf("\nPriority order: ");
    while (pq_pop(&pq, &e)) printf("%s, ", e.name);
    printf("\n");
    return 0;
}
