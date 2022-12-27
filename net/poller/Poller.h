#pragma once

/* Standard headers */
#include <vector>
#include <map>

/* Local headers */
#include "utils/NonCopyable.h"
#include "utils/Timestamp.h"

class EventLoop;
class Channel;

class Poller : public NonCopyable {
public:
    using ChannelList = std::vector<Channel*>;

public:
    Poller(EventLoop& loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(ChannelList&, int) = 0;
    virtual void updateChannel(Channel&) = 0;
    virtual void removeChannel(Channel&) = 0;

protected:
    using ChannelMap = std::map<int, Channel*>;

protected:
    EventLoop& loop_;
    ChannelMap channels_;
};