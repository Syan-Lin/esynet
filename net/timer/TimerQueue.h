#pragma once

/* Standard headers */
#include <vector>
#include <map>

/* Local headers */
#include "net/timer/Timer.h"
#include "net/base/Event.h"

namespace esynet::timer {

/* 负责定时器任务的注册、回调、管理，不保证回调
 * 函数一定会准时执行，有可能会因为繁忙而延后 */
class TimerQueue {
public:
    using TimerPtr = std::unique_ptr<Timer>;
    using TimerList = std::vector<Timer*>;
    using TimerMap = std::map<Timestamp, TimerPtr>;   /* 有序表，按到期时间从小到大排序 */
    using TimerPositionMap = std::map<Timer::ID, Timestamp>;

public:
    TimerQueue(Reactor&);
    ~TimerQueue();

    Timer::ID addTimer(Timer::Callback, Timestamp expiration, double interval);
    void cancel(Timer::ID);

    /* 由 Reactor 调用 */
    void handle();

private:
    TimerList getExpired(Timestamp now) const;    /* 获取所有超时事件 */
    int createTimerFd();
    void updateTimerFd();

private:
    TimerMap timerMap_;
    TimerPositionMap timerPositionMap_; /* 记录Timer在timerMap_中的位置用于取消 */
    int timerFd_;
    Reactor& reactor_;
    Event timerEvent_;
};

} /* namespace esynet::timer */