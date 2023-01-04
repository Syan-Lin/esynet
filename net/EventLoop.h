#pragma once

/* Standard headers */
#include <thread>
#include <memory>
#include <vector>
#include <sstream>
#include <mutex>

/* Local headers */
#include "net/timer/TimerQueue.h"
#include "utils/NonCopyable.h"
#include "utils/Timestamp.h"
#include "net/poller/Poller.h"
#include "net/timer/Timer.h"

namespace esynet {

class Event;

/* EventLoop 管理一个 Poller 以及数个 Event
 * Poller 以及 Event 都和所属的 EventLoop
 * 在同一个线程。同时每个线程最多拥有一个 EventLoop
 * 所以这些类都不必是线程安全的，不会发生跨线程调用
 *
 *
 * 注意：
 *      1.没有特殊说明，则代表非线程安全
 *      2.不要设为全局变量                      */

class EventLoop : public utils::NonCopyable {
public:
    static EventLoop* getEvtlpOfCurThread();
    static int kPollTimeMs;

private:
    using EventList = std::vector<Event*>;
    using Timer = timer::Timer;
    using TimerQueue = timer::TimerQueue;
    using Poller = poller::Poller;
    using Timestamp = utils::Timestamp;
    std::string tidToStr() const;

public:
    EventLoop(bool = true);
    ~EventLoop();

    void loop();
    void stop();    /* 线程安全 */

    Timestamp lastPollTime() const; /* TODO: 需要线程安全？ */
    void updateEvent(Event&);

    /* Timer 相关：线程安全 */
    Timer::ID runAt(Timestamp, Timer::Callback);
    Timer::ID runAfter(double, Timer::Callback);
    Timer::ID runEvery(double, Timer::Callback);

    bool isInLoopThread() const;    /* 线程安全 */
    bool isLoopping() const;        /* 线程安全 */

private:
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    EventList activeEvents_;

    Timestamp lastPollTime_;
    std::atomic<bool> stop_;
    std::atomic<bool> isLoopping_;
    const std::thread::id tid_;
};

} /* namespace esynet */