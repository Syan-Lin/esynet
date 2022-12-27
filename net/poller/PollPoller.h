#pragma once

/* Standard headers */
#include <vector>

/* Linux headers */
#include <sys/poll.h>

/* Local headers */
#include "utils/Timestamp.h"
#include "logger/Logger.h"
#include "net/poller/Poller.h"

class Channel;
class EventLoop;

class PollPoller : public Poller {
public:
    PollPoller(EventLoop& loop);
    ~PollPoller() override = default;

    Timestamp poll(ChannelList&, int) override;
    void updateChannel(Channel&) override;
    void removeChannel(Channel&) override;

private:
    using PollFd = struct pollfd;
    using PollFdList = std::vector<PollFd>;

private:
    void fillActiveChannels(int, ChannelList&) const;
    PollFdList pollfds_;
};