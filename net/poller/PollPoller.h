#pragma once

/* Linux headers */
#include <sys/poll.h>

/* Local headers */
#include "net/poller/Poller.h"

namespace esynet::poller {

class PollPoller : public Poller {
public:
    PollPoller(EventLoop& loop);
    ~PollPoller() override = default;

    utils::Timestamp poll(EventList&, int timeoutMs) override;
    void updateEvent(Event&) override;
    void removeEvent(Event&) override;

private:
    using PollFd = struct pollfd;
    using PollFdList = std::vector<PollFd>;

private:
    void fillActiveEvents(int numEvents, EventList&) const;
    PollFdList pollFds_;
};

} /* namespace esynet::poller */