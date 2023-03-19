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

    utils::Timestamp poll(EventList&, int timeoutMs) override;
    void updateEvent(Event&) override;
    void removeEvent(Event&) override;

private:
    using PollFd = struct pollfd;

private:
    void fillActiveEvents(int numEvents, EventList&) const;
    std::vector<PollFd> pollFds_;
};

} /* namespace esynet::poller */