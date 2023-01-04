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
    EpollPoller(EventLoop&);
    ~EpollPoller() override;

    utils::Timestamp poll(EventList&, int) override;
    void updateEvent(Event&) override;
    void removeEvent(Event&) override;

private:
    using EpollEvent = struct epoll_event;
    using EpollEventList = std::vector<EpollEvent>;

private:
    void epollUpdate(int, Event&);
    void fillActiveEvents(int, EventList&) const;
    EpollEventList epollEvents_;
    int epollFd_;
};

} /* namespace esynet::poller */