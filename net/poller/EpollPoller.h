#pragma once

/* Linux headers */
#include <sys/epoll.h>

/* Local headers */
#include "net/poller/Poller.h"
#include "net/Channel.h"
#include "logger/Logger.h"

class EpollPoller : public Poller {
public:
    static const int kInitEventListSize;
public:
    EpollPoller(EventLoop&);
    ~EpollPoller() override;

    Timestamp poll(ChannelList&, int) override;
    void updateChannel(Channel&) override;
    void removeChannel(Channel&) override;

private:
    using EpollEvent = struct epoll_event;
    using EpollEventList = std::vector<EpollEvent>;

private:
    void epollUpdate(int, Channel&);
    void fillActiveChannels(int, ChannelList&) const;
    EpollEventList epollEvents_;
    int epollFd_;
};