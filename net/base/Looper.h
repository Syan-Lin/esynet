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

/* 警告： 不要设为全局变量 */
class Looper : public utils::NonCopyable {
public:
    static const int kPollTimeMs;

private:
    using EventList  = std::vector<Event*>;
    using Timer      = timer::Timer;
    using TimerQueue = timer::TimerQueue;
    using Poller     = poller::Poller;
    using Timestamp  = utils::Timestamp;
    using Function   = std::function<void()>;

    bool isInLoopThread() const;
    void wakeup();

public:
    Looper(bool useEpoll = true);
    ~Looper();

    void start();
    void stop();

    auto lastPollTime() -> Timestamp;
    void updateEvent(Event&);
    void removeEvent(Event&);

    auto runAt(Timestamp timePoint, Timer::Callback) -> Timer::ID;
    auto runAfter(double delay, Timer::Callback) -> Timer::ID;
    auto runEvery(double interval, Timer::Callback) -> Timer::ID;
    void cancelTimer(Timer::ID);

    /* 由该 Looper 所属的线程来调用传入函数 */
    void run(Function);       /* 立刻唤醒执行 */
    void queue(Function);     /* 等待唤醒，稍后执行 */

    void assert() const;
    bool isLooping() const;
    int numOfEvents();

private:
    EventList activeEvents_;
    std::vector<Function> tasks_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    /* 状态 */
    Timestamp lastPollTime_;
    const std::thread::id tid_;
    std::atomic<int>  numOfEvents_ {0};
    std::atomic<bool> stop_        {false};
    std::atomic<bool> isLooping_   {false};

    /* 多线程 */
    int wakeupFd_;
    std::mutex mutex_;
    Event wakeupEvent_;
};

} /* namespace esynet */