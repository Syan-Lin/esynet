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
class Reactor : public utils::NonCopyable {
public:
    static const int kPollTimeMs;

private:
    using EventList  = std::vector<Event*>;
    using Timer      = timer::Timer;
    using TimerQueue = timer::TimerQueue;
    using Poller     = poller::Poller;
    using Timestamp  = utils::Timestamp;
    using Function   = std::function<void()>;

    std::string tidToStr(const std::thread::id) const;
    bool isInLoopThread() const;
    void wakeup();

public:
    Reactor(bool useEpoll = true);
    ~Reactor();

    void start();
    void stop();

    Timestamp lastPollTime();
    void updateEvent(Event&);
    void removeEvent(Event&);

    Timer::ID runAt(Timestamp timePoint, Timer::Callback);
    Timer::ID runAfter(double delay, Timer::Callback);
    Timer::ID runEvery(double interval, Timer::Callback);
    void cancelTimer(Timer::ID);

    /* 由该 Reactor 所属的线程来调用传入函数 */
    void run(Function);       /* 立刻唤醒执行 */
    void queue(Function);     /* 等待唤醒，稍后执行 */

    void assert() const;
    bool isLooping() const;
    int numOfEvents();

private:
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    std::vector<Function> tasks_;
    EventList activeEvents_;

    /* 状态 */
    Timestamp lastPollTime_;
    std::atomic<bool> stop_;
    std::atomic<bool> isLooping_;
    std::atomic<int> numOfEvents_;
    const std::thread::id tid_;

    /* 多线程 */
    std::mutex mutex_;
    int wakeupFd_;
    Event wakeupEvent_;
};

} /* namespace esynet */