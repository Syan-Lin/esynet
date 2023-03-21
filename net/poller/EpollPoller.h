#pragma once

/* Linux headers */
#include <sys/epoll.h>

/* Local headers */
#include "net/poller/Poller.h"

namespace esynet::poller {

class EpollPoller : public Poller {
public:
    static const int kInitEventListSize;
public:
    EpollPoller(Looper&);
    ~EpollPoller() override;

    auto poll(EventList&, int timeoutMs) -> utils::Timestamp override;
    void updateEvent(Event&) override;
    void removeEvent(Event&) override;

private:
    void epollUpdate(int operation, Event&);
    void fillActiveEvents(int eventsNum, EventList&) const;

private:
    using EpollEvent = struct epoll_event;

    int epollFd_;
    std::vector<EpollEvent> epollEvents_;
};

} /* namespace esynet::poller */