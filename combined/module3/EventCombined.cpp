#include "EventCombined.h"
#include "../common/json.h"

using namespace std;

void EventCombined::reset()
{
    queue_init(&fifo);
    pq_init(&pq);
    history_init(&history);
    nextEventId = 1;
}

bool EventCombined::enqueue(int type, bool usePriorityQueue)
{
    Event* e = createEvent(type);
    if (e == 0) return false;

    EventRec rec;
    rec.id = nextEventId;
    rec.type = type;
    rec.priority = e->getPriority();
    rec.day = eco.getDay();
    eco_copy_name(rec.name, e->getName().c_str());
    delete e;

    bool added = usePriorityQueue ? pq_push(&pq, rec) : queue_enqueue(&fifo, rec);
    if (added) nextEventId++;
    return added;
}

bool EventCombined::processRecord(const EventRec& rec)
{
    Event* e = createEvent(rec.type);
    if (e == 0) return false;
    string effect = eco.applyEvent(*e);
    history_add(&history, rec, eco.getDay(), effect.c_str());
    delete e;
    return true;
}

int EventCombined::processAll()
{
    EventRec rec;
    int count = 0;
    while (pq_pop(&pq, &rec))        { if (processRecord(rec)) count++; }
    while (queue_dequeue(&fifo, &rec)) { if (processRecord(rec)) count++; }
    return count;
}

static string recJson(const EventRec& e)
{
    return "{\"id\":" + json::num(e.id) + ",\"type\":" + json::num(e.type) + ",\"name\":" + json::str(e.name) +
           ",\"priority\":" + json::num(e.priority) + ",\"day\":" + json::num(e.day) + "}";
}

string EventCombined::addJson(int type, const string& kind)
{
    bool usePQ = (kind == "priority");
    if (kind != "priority" && kind != "fifo") return json::error("Queue kind must be 'fifo' or 'priority'");
    if (!enqueue(type, usePQ)) return json::error("Could not add event (unknown type or queue full)");
    return json::ok("\"message\":" + json::str(string("Event added to the ") + (usePQ ? "priority queue" : "FIFO queue")));
}

string EventCombined::processNextJson(const string& kind)
{
    EventRec rec;
    bool got = (kind == "priority") ? pq_pop(&pq, &rec) : queue_dequeue(&fifo, &rec);
    if (!got) return json::error(string("The ") + (kind == "priority" ? "priority queue" : "FIFO queue") + " is empty");
    processRecord(rec);
    const HistoryRec& h = history.items[history.count - 1];
    return json::ok("\"processed\":" + recJson(rec) + ",\"effect\":" + json::str(h.effect));
}

string EventCombined::processAllJson()
{
    int n = processAll();
    return json::ok("\"processedCount\":" + json::num(n));
}

string EventCombined::eventsJson() const
{
    EventRec tmp[MAX_EVENTS];
    string a, b, h, types;

    int n = queue_list(&fifo, tmp);
    for (int i = 0; i < n; i++) { if (i) a += ","; a += recJson(tmp[i]); }

    n = pq_list(&pq, tmp);
    for (int i = 0; i < n; i++) { if (i) b += ","; b += recJson(tmp[i]); }

    for (int i = history.count - 1; i >= 0; i--) {
        const HistoryRec& r = history.items[i];
        if (!h.empty()) h += ",";
        h += "{\"id\":" + json::num(r.ev.id) + ",\"name\":" + json::str(r.ev.name) +
             ",\"priority\":" + json::num(r.ev.priority) + ",\"addedDay\":" + json::num(r.ev.day) +
             ",\"processedDay\":" + json::num(r.processedDay) + ",\"effect\":" + json::str(r.effect) + "}";
    }

    for (int t = 1; t <= 5; t++) {
        Event* e = createEvent(t);
        if (t > 1) types += ",";
        types += "{\"type\":" + json::num(t) + ",\"name\":" + json::str(e->getName()) +
                 ",\"priority\":" + json::num(e->getPriority()) + ",\"description\":" + json::str(e->description()) + "}";
        delete e;
    }

    return json::ok("\"fifo\":[" + a + "],\"priorityQueue\":[" + b + "],\"history\":[" + h + "],\"types\":[" + types + "]");
}

static string historyItemJson(const HistoryRec& r)
{
    return "{\"name\":" + json::str(r.ev.name) + ",\"day\":" + json::num(r.processedDay) +
           ",\"effect\":" + json::str(r.effect) + "}";
}

string EventCombined::lastEventJson() const
{
    if (history.count == 0) return "null";
    return historyItemJson(history.items[history.count - 1]);
}

string EventCombined::markersJson() const
{
    string out;
    for (int i = 0; i < history.count; i++) {
        if (i > 0) out += ",";
        out += "{\"day\":" + json::num(history.items[i].processedDay) + ",\"name\":" + json::str(history.items[i].ev.name) + "}";
    }
    return "[" + out + "]";
}

string EventCombined::eventsSinceJson(int fromIndex) const
{
    string out;
    for (int i = fromIndex; i < history.count; i++) {
        if (i > fromIndex) out += ",";
        out += historyItemJson(history.items[i]);
    }
    return "[" + out + "]";
}
