#include "net/timer/TimerQueue.h"

/* Linux headers */
#include <sys/timerfd.h>

/* Local headers */
#include "net/base/Looper.h"
#include "logger/Logger.h"
#include "utils/ErrorInfo.h"

using esynet::timer::TimerQueue;
using esynet::timer::Timer;

int TimerQueue::createTimerFd() {
    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0) {
        LOG_FATAL("Failed to create timerfd(err: {})", errnoStr(errno));
    }
    return timerfd;
}

/* 将超时时间更新为最近的超时时间 */
void TimerQueue::updateTimerFd() {
    struct itimerspec closeTime;
    bzero(&closeTime, sizeof closeTime);

    if(!timerMap_.empty()) {    /* 精度到微秒 */
        auto latest = timerMap_.begin()->first;
        closeTime.it_value.tv_sec = latest.secondsSinceEpoch();
        closeTime.it_value.tv_nsec = latest.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond * 1000;
    }
    if(timerfd_settime(timerFd_, TFD_TIMER_ABSTIME, &closeTime, nullptr) == -1) {
        LOG_ERROR("Failed to set timerfd(err: {})", errnoStr(errno));
    }
}

TimerQueue::TimerQueue(Looper& looper):
                looper_(looper),
                timerFd_(createTimerFd()),
                timerEvent_(looper, timerFd_) {
    timerEvent_.setReadCallback(std::bind(&TimerQueue::handle, this));
    timerEvent_.enableRead();
}
TimerQueue::~TimerQueue() {
    looper_.removeEvent(timerEvent_);
    close(timerFd_);
}

Timer::ID TimerQueue::addTimer(Timer::Callback callback, Timestamp expiration, double interval) {
    TimerPtr tp = std::make_unique<Timer>(callback, expiration, interval);
    Timer::ID id = tp->id();

    looper_.run([this, expiration, &tp] {
        timerMap_[expiration] = std::move(tp);
        timerPositionMap_[timerMap_[expiration]->id()] = expiration;
        updateTimerFd();
    });

    return id;
}
void TimerQueue::cancel(Timer::ID id) {
    looper_.run([this, id] {
        auto iter = timerPositionMap_.find(id);
        if(iter != timerPositionMap_.end()) {
            timerMap_.erase(iter->second);
            timerPositionMap_.erase(id);
            updateTimerFd();
        }
    });
}

void TimerQueue::handle() {
    TimerList expiredList = getExpired(Timestamp::now());
    LOG_DEBUG("{} timers expired", expiredList.size());
    for(auto& timer : expiredList) {
        timer->run();

        /* 维护 timerMap_ 和 timerPositionMap_
         * 删除过期 Timer 以及重新注册周期 Timer */
        TimerPtr temp = std::move(timerMap_[timer->expiration()]);
        timerMap_.erase(temp->expiration());
        if(temp->repeat()) {
            temp->restart();
            timerPositionMap_[temp->id()] = temp->expiration();
            timerMap_[temp->expiration()] = std::move(temp);
        } else {
            timerPositionMap_.erase(temp->id());
        }
    }
    updateTimerFd();
}
TimerQueue::TimerList TimerQueue::getExpired(Timestamp now) const {
    TimerList expiredList;
    auto iter = timerMap_.begin();
    for(; iter != timerMap_.end() && !(iter->first > now); ++iter) {
        expiredList.push_back(iter->second.get());
    }
    return expiredList;
}