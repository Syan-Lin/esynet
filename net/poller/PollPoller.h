#pragma once

/* Linux headers */
#include <sys/poll.h>

/* Local headers */
#include "net/poller/Poller.h"

namespace esynet::poller {

class PollPoller : public Poller {
public:
    PollPoller(Looper& looper);
    ~PollPoller() override = default;

    auto poll(EventList&, int timeoutMs) -> utils::Timestamp override;
    void updateEvent(Event&) override;
    void removeEvent(Event&) override;

private:
    void fillActiveEvents(int eventsNum, EventList&) const;

private:
    using PollFd = struct pollfd;

    std::vector<PollFd> pollFds_;
};

} /* namespace esynet::poller */