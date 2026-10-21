#ifndef EVENT_COMBINED_H
#define EVENT_COMBINED_H

#include <string>
#include "../../c_modules/module3_events/event_queue.h"
#include "../../cpp_modules/module5_ecosystem/Ecosystem.h"

class EventCombined {
    EventQueue&   fifo;
    EventPQ&      pq;
    EventHistory& history;
    Ecosystem&    eco;
    int           nextEventId;
public:
    EventCombined(EventQueue& f, EventPQ& p, EventHistory& h, Ecosystem& e)
        : fifo(f), pq(p), history(h), eco(e), nextEventId(1) {}

    void reset();

    bool enqueue(int type, bool usePriorityQueue);
    bool processRecord(const EventRec& rec);
    int  processAll();
    int  pendingCount() const { return fifo.count + pq.count; }
    int  historyCount() const { return history.count; }

    std::string addJson(int type, const std::string& kind);
    std::string processNextJson(const std::string& kind);
    std::string processAllJson();
    std::string eventsJson() const;
    std::string lastEventJson() const;
    std::string markersJson() const;
    std::string eventsSinceJson(int fromIndex) const;
};

#endif
