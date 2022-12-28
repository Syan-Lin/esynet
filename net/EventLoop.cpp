#include "net/EventLoop.h"
#include "net/Event.h"
// #include "logger/Logger.h"
#include "net/poller/PollPoller.h"
#include "net/poller/EpollPoller.h"

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
        // LOG_FATAL("Another EventLoop({:p}) exists in this thread({})",
        //             static_cast<void*>(t_evtlpInCurThread), tidToStr());
    } else {
        t_evtlpInCurThread = this;
        // LOG_DEBUG("EventLoop({:p}) created in thread {}",
        //             static_cast<void*>(this), tidToStr());
    }
    if(useEpoll) {
        poller_ = std::make_unique<EpollPoller>(*this);
    } else {
        poller_ = std::make_unique<PollPoller>(*this);
    }
}

EventLoop::~EventLoop() {
    if(isLoopping_) {
        // LOG_FATAL("Deconstruct EventLoop({:p}) while is loopping",
        //             static_cast<void*>(t_evtlpInCurThread));
    }
    t_evtlpInCurThread = nullptr;
}

void EventLoop::loop() {
    if(!isInLoopThread()) {
        // LOG_FATAL("Try to do loop({:p}) in another thread", static_cast<void*>(this));
    }
    isLoopping_ = true;
    while(!stop_) {
        activeEvents_.clear();
        /* 获取活动事件 */
        poller_->poll(activeEvents_, kPollTimeMs);
        /* 执行活动事件对应的回调函数 */
        for(auto& event : activeEvents_) {
            event->handle();
        }
    }
    isLoopping_ = false;
}
void EventLoop::stop() { stop_ = true; }

/* 将Event委托的update转发给poller */
void EventLoop::updateEvent(Event& event) {
    if(event.ownerLoop() != this) {
        // LOG_FATAL("Event({}) doesn't belong to loop({:p})",
        //     event.fd(), static_cast<void*>(this));
    }
    poller_->updateEvent(event);
}

inline bool EventLoop::isInLoopThread() const {
    return tid_ == std::this_thread::get_id();
}

std::string EventLoop::tidToStr() const {
    std::stringstream ss;
    ss << tid_;
    std::string tidStr = ss.str();
    tidStr = tidStr.substr(tidStr.length() - 4, 4);
    return tidStr;
}