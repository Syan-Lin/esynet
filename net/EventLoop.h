#pragma once

/* Standard headers */
#include <thread>
#include <memory>
#include <vector>
#include <sstream>

/* Local headers */
#include "utils/NonCopyable.h"
#include "net/poller/Poller.h"

class Event;

/* EventLoop 管理一个 Poller 以及数个 Event
 * Poller 以及 Event 都和所属的 EventLoop
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
    void updateEvent(Event&);

    bool isInLoopThread() const;

private:
    using EventList = std::vector<Event*>;
    std::string tidToStr() const;

private:
    std::unique_ptr<Poller> poller_;
    EventList activeEvents_;
    bool stop_;
    bool isLoopping_;
    const std::thread::id tid_;
};