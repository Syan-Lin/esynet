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

/* Reactor 管理一个 Poller 以及数个 Event
 * Poller 以及 Event 都和所属的 Reactor
 * 在同一个线程。同时每个线程最多拥有一个 Reactor
 * 所以这些类都不必是线程安全的，不会发生跨线程调用
 *
 * Reactor 大部分函数都是线程安全的，非线程安全会
 * 特殊标注
 *
 * 注意：不要设为全局变量 */

class Reactor : public utils::NonCopyable {
public:
    static Reactor* getReactorOfCurThread();
    static const int kPollTimeMs;

private:
    using EventList = std::vector<Event*>;
    using Timer = timer::Timer;
    using TimerQueue = timer::TimerQueue;
    using Poller = poller::Poller;
    using Timestamp = utils::Timestamp;
    using Function = std::function<void()>;

    std::string tidToStr() const;
    void wakeup();

public:
    Reactor(bool = true);
    ~Reactor();

    void start();    /* 不允许跨线程调用 */
    void stop();

    Timestamp lastPollTime();
    void updateEvent(Event&);   /* 非线程安全 */
    void removeEvent(Event&);   /* 非线程安全 */

    Timer::ID runAt(Timestamp timePoint, Timer::Callback);
    Timer::ID runAfter(double delay, Timer::Callback);
    Timer::ID runEvery(double interval, Timer::Callback);
    void cancelTimer(Timer::ID);

    /* 由该Reactor所属的线程来调用传入函数 */
    void run(Function);       /* 立刻唤醒执行 */
    void queue(Function);     /* 等待唤醒，稍后执行 */

    bool isInLoopThread() const;
    bool isLooping() const;
    int numOfEvents();

private:
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    EventList activeEvents_;
    std::vector<Function> tasks_;

    /* 状态相关 */
    Timestamp lastPollTime_;
    std::atomic<bool> stop_;
    std::atomic<bool> isLooping_;
    std::atomic<int> numOfEvents_;
    const std::thread::id tid_;

    /* 多线程相关 */
    std::mutex mutex_;
    int wakeupFd_;
    Event wakeupEvent_;
};

} /* namespace esynet */