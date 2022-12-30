#include "net/timer/TimerQueue.h"

/* Linux headers */
#include <sys/timerfd.h>

#include <dbg.h>

/* Local headers */
#include "net/EventLoop.h"
#include "logger/Logger.h"

int TimerQueue::createTimerFd() {
    int timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0) {
        LOG_ERROR("Failed to create timerfd({})", strerror(errno));
        abort();
    }
    return timerfd;
}
void TimerQueue::updateTimerFd() {
    struct itimerspec closeTime;
    bzero(&closeTime, sizeof closeTime);

    if(!timerMap_.empty()) {    /* 精度到微秒 */
        auto latest = timerMap_.begin()->first;
        closeTime.it_value.tv_sec = latest.secondsSinceEpoch();
        closeTime.it_value.tv_nsec = latest.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond * 1000;
    }
    int ret = timerfd_settime(timerFd_, TFD_TIMER_ABSTIME, &closeTime, nullptr);
    if(ret) {
        LOG_ERROR("Failed to set timerfd({})", strerror(errno));
    }
}

TimerQueue::TimerQueue(EventLoop& loop)
        : loop_(loop), timerFd_(createTimerFd()), timerEvent_(loop, timerFd_) {
    timerEvent_.setReadCallback(std::bind(&TimerQueue::handle, this));
    timerEvent_.enableReading();
}
TimerQueue::~TimerQueue() {
    timerEvent_.disableAll();
    close(timerFd_);
}

Timer::ID TimerQueue::addTimer(Timer::Callback callback, Timestamp when, double interval) {
    timerMap_[when] = std::make_unique<Timer>(callback, when, interval);
    timerPositionMap_[timerMap_[when]->id()] = when;
    updateTimerFd();
    return timerMap_[when]->id();
}
void TimerQueue::cancel(Timer::ID id) {
    auto iter = timerPositionMap_.find(id);
    if(iter != timerPositionMap_.end()) {
        timerMap_.erase(iter->second);
        timerPositionMap_.erase(id);
        updateTimerFd();
    }
}

void TimerQueue::handle() {
    TimerList expiredList = getExpired(Timestamp::now());
    LOG_DEBUG("{} timers expired", expiredList.size());
    for(auto& timer : expiredList) {
        timer->run();

        /* 维护timerMap_和timerPositionMap_
         * 删除过期Timer以及重新注册周期Timer */
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
TimerQueue::TimerList TimerQueue::getExpired(Timestamp now) {
    TimerList expiredList;
    auto iter = timerMap_.begin();
    for(; iter != timerMap_.end() && !(iter->first > now); ++iter) {
        expiredList.push_back(iter->second.get());
    }
    return expiredList;
}