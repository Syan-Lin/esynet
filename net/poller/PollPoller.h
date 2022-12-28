#pragma once

/* Linux headers */
#include <sys/poll.h>

/* Local headers */
#include "net/poller/Poller.h"
#include "net/Event.h"

class PollPoller : public Poller {
public:
    PollPoller(EventLoop& loop);
    ~PollPoller() override = default;

    Timestamp poll(EventList&, int) override;
    void updateEvent(Event&) override;
    void removeEvent(Event&) override;

private:
    using PollFd = struct pollfd;
    using PollFdList = std::vector<PollFd>;

private:
    void fillActiveEvents(int, EventList&) const;
    PollFdList pollfds_;
};