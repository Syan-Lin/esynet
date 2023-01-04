#include "net/EventLoop.h"

/* Local headers */
#include "net/Event.h"
#include "logger/Logger.h"
#include "net/poller/PollPoller.h"
#include "net/poller/EpollPoller.h"
#include "net/timer/TimerQueue.h"
#include "utils/Timestamp.h"

using esynet::EventLoop;
using esynet::poller::EpollPoller;
using esynet::poller::PollPoller;
using esynet::utils::Timestamp;
using esynet::timer::Timer;

/* 每个线程至多有一个 EventLoop */
thread_local EventLoop* t_evtlpInCurThread = nullptr;
/* 超时时间 */
int EventLoop::kPollTimeMs = 3000;

EventLoop* EventLoop::getEvtlpOfCurThread() {
    return t_evtlpInCurThread;
}

EventLoop::EventLoop(bool useEpoll)
        : tid_(std::this_thread::get_id()),
          isLoopping_(false), stop_(false) {
    if(t_evtlpInCurThread) {
        LOG_FATAL("Another EventLoop({:p}) exists in this thread({})",
                    static_cast<void*>(t_evtlpInCurThread), tidToStr());
    } else {
        t_evtlpInCurThread = this;
        LOG_DEBUG("EventLoop({:p}) created in thread {}",
                    static_cast<void*>(this), tidToStr());
    }
    if(useEpoll) {
        poller_ = std::make_unique<EpollPoller>(*this);
    } else {
        poller_ = std::make_unique<PollPoller>(*this);
    }
    timerQueue_ = std::make_unique<timer::TimerQueue>(*this);
}

EventLoop::~EventLoop() {
    if(isLoopping_) {
        LOG_FATAL("Deconstruct EventLoop({:p}) while is loopping",
                    static_cast<void*>(t_evtlpInCurThread));
    }
    t_evtlpInCurThread = nullptr;
}

void EventLoop::loop() {
    if(!isInLoopThread()) {
        LOG_FATAL("Try to do loop({:p}) in another thread", static_cast<void*>(this));
    }
    stop_ = false;
    isLoopping_ = true;
    LOG_DEBUG("EventLoop({:p}) start looping", static_cast<void*>(this));
    while(!stop_) {
        activeEvents_.clear();
        /* 获取活动事件 */
        lastPollTime_ = poller_->poll(activeEvents_, kPollTimeMs);
        /* 执行活动事件对应的回调函数 */
        for(auto& event : activeEvents_) {
            event->handle();
        }
    }
    LOG_DEBUG("EventLoop({:p}) stop looping", static_cast<void*>(this));
    isLoopping_ = false;
}
void EventLoop::stop() { stop_ = true; }
Timestamp EventLoop::lastPollTime() const { return lastPollTime_; }

Timer::ID EventLoop::runAt(Timestamp timePoint, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, timePoint, 0.0);
}

Timer::ID EventLoop::runAfter(double time, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now() + time, 0.0);
}

Timer::ID EventLoop::runEvery(double time, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now(), time);
}

/* 将Event委托的update转发给poller */
void EventLoop::updateEvent(Event& event) {
    if(event.ownerLoop() != this) {
        LOG_FATAL("Event({}) doesn't belong to loop({:p})",
            event.fd(), static_cast<void*>(this));
    }
    poller_->updateEvent(event);
}

bool EventLoop::isInLoopThread() const {
    return tid_ == std::this_thread::get_id();
}
bool EventLoop::isLoopping() const { return isLoopping_; }

std::string EventLoop::tidToStr() const {
    std::stringstream ss;
    ss << tid_;
    std::string tidStr = ss.str();
    tidStr = tidStr.substr(tidStr.length() - 4, 4);
    return tidStr;
}