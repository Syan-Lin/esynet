#pragma once

/* Standard headers */
#include <vector>
#include <map>

/* Local headers */
#include "utils/NonCopyable.h"
#include "utils/Timestamp.h"

class EventLoop;
class Event;

class Poller : public NonCopyable {
public:
    using EventList = std::vector<Event*>;

public:
    Poller(EventLoop& loop): loop_(loop) {};
    virtual ~Poller() = default;

    virtual Timestamp poll(EventList&, int) = 0;
    virtual void updateEvent(Event&) = 0;
    virtual void removeEvent(Event&) = 0;

protected:
    using EventMap = std::map<int, Event*>;

protected:
    EventLoop& loop_;
    EventMap events_;
};