#pragma once

/* Standard headers */
#include <vector>
#include <map>

/* Local headers */
#include "net/timer/Timer.h"
#include "net/Event.h"

class EventLoop;

/* 负责定时器任务的注册、回调、管理，非线程安全
 * 不保证回调函数一定会准时执行，有可能会因为
 * 繁忙而延后 */
class TimerQueue {
public:
    using TimerPtr = std::unique_ptr<Timer>;
    using TimerList = std::vector<Timer*>;
    using TimerMap = std::map<Timestamp, TimerPtr>;   /* 有序表，按到期时间从小到大排序 */
    using TimerPositionMap = std::map<Timer::ID, Timestamp>;

public:
    TimerQueue(EventLoop&);
    ~TimerQueue();

    Timer::ID addTimer(Timer::Callback, Timestamp, double);
    void cancel(Timer::ID);

    void handle();

private:
    TimerList getExpired(Timestamp);    /* 获取所有超时事件 */
    int createTimerFd();
    void updateTimerFd();

private:
    TimerMap timerMap_;
    TimerPositionMap timerPositionMap_; /* 记录Timer在timerMap_中的位置用于取消 */
    int timerFd_;
    EventLoop& loop_;
    Event timerEvent_;
};