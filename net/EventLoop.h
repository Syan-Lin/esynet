#pragma once

/* Standard headers */
#include <thread>
#include <sstream>
#include <memory>

/* Local headers */
#include "utils/NonCopyable.h"
#include "logger/Logger.h"
#include "net/Channel.h"
#include "net/poller/Poller.h"
#include "net/poller/PollPoller.h"
#include "net/poller/EpollPoller.h"

/* EventLoop 管理一个 Poller 以及数个 Channel
 * Poller 以及 Channel 都和所属的 EventLoop
 * 在同一个线程。同时每个线程最多拥有一个 EventLoop
 * 所以这些类都不必是线程安全的，不会发生跨线程调用 */

class EventLoop : public NonCopyable {
public:
    static EventLoop* getEvtlpOfCurThread();
    static int kPollTimeMs;
public:
    EventLoop(bool = true);
    ~EventLoop();

    void loop();
    void stop();
    void updateChannel(Channel&);

    bool isInLoopThread() const;

private:
    using ChannelList = Poller::ChannelList;
    std::string tidToStr() const;

private:
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
    bool stop_;
    bool isLoopping_;
    const std::thread::id tid_;
};