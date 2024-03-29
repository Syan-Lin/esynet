#pragma once

/* Standard headers */
#include <vector>
#include <map>

/* Local headers */
#include "utils/NonCopyable.h"
#include "utils/Timestamp.h"

namespace esynet {

class Looper;
class Event;

}

namespace esynet::poller {

class Poller : public utils::NonCopyable {
public:
    using EventList = std::vector<Event*>;

public:
    Poller(Looper& looper): looper_(looper) {};
    virtual ~Poller() = default;

    virtual auto poll(EventList&, int timeoutMs) -> utils::Timestamp = 0;
    virtual void updateEvent(Event&) = 0;
    virtual void removeEvent(Event&) = 0;

protected:
    using EventMap = std::map<int, Event*>;

    Looper& looper_;
    EventMap events_;
};

} /* namespace esynet::poller */