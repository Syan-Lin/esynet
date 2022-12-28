#pragma once

/* Linux headers */
#include <sys/epoll.h>

/* Local headers */
#include "net/poller/Poller.h"

class EpollPoller : public Poller {
public:
    static const int kInitEventListSize;
public:
    EpollPoller(EventLoop&);
    ~EpollPoller() override;

    Timestamp poll(EventList&, int) override;
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