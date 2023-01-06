#include "net/EventLoop.h"

/* Local headers */
#include "net/Event.h"
#include "logger/Logger.h"
#include "net/poller/PollPoller.h"
#include "net/poller/EpollPoller.h"
#include "net/timer/TimerQueue.h"
#include "utils/Timestamp.h"

/* Linux headers */
#include <sys/eventfd.h>

using esynet::EventLoop;
using esynet::poller::EpollPoller;
using esynet::poller::PollPoller;
using esynet::utils::Timestamp;
using esynet::timer::Timer;
using esynet::Logger;

/* 每个线程至多有一个 EventLoop */
thread_local EventLoop* t_evtlpInCurThread = nullptr;
/* 超时时间 */
int EventLoop::kPollTimeMs = 3000;

EventLoop* EventLoop::getEvtlpOfCurThread() {
    return t_evtlpInCurThread;
}

int createEventFd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd == -1) {
        LOG_ERROR("Failed to create eventfd({})", strerror(errno));
    }
    return fd;
}

EventLoop::EventLoop(bool useEpoll)
        : stop_(false),
          numOfEvents_(0),
          isLoopping_(false),
          wakeupFd_(createEventFd()),
          wakeupEvent_(*this, wakeupFd_),
          tid_(std::this_thread::get_id()) {
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

    /* 注册poll唤醒事件 */
    wakeupEvent_.setReadCallback([this] {
        uint64_t temp;
        if(eventfd_read(wakeupFd_, &temp) == -1) {
            LOG_ERROR("Failed to read eventfd({})", strerror(errno));
        }
        LOG_DEBUG("Wake up EventLoop({:p})", static_cast<void*>(this));
    });
    wakeupEvent_.enableReading();
}

EventLoop::~EventLoop() {
    if(isLoopping_) {
        LOG_FATAL("Deconstruct EventLoop({:p}) while is loopping",
                    static_cast<void*>(t_evtlpInCurThread));
    }
    t_evtlpInCurThread = nullptr;
    wakeupEvent_.disableAll();
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
        auto temp = poller_->poll(activeEvents_, kPollTimeMs);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            lastPollTime_ = temp;
        }
        /* 执行活动事件对应的回调函数 */
        for(auto& event : activeEvents_) {
            event->handle();
        }

        /* 执行任务队列 */
        std::vector<Function> tasks;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks.swap(tasks_);
        }
        for(auto& task : tasks) {
            task();
        }
    }
    LOG_DEBUG("EventLoop({:p}) stop looping", static_cast<void*>(this));
    isLoopping_ = false;
}
void EventLoop::stop() { stop_ = true; wakeup(); }
Timestamp EventLoop::lastPollTime() {
    std::unique_lock<std::mutex> lock(mutex_);
    return lastPollTime_;
}

/* 该部分的线程安全由TimerQueue保证 */
Timer::ID EventLoop::runAt(Timestamp timePoint, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, timePoint, 0.0);
}
Timer::ID EventLoop::runAfter(double time, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now() + time, 0.0);
}
Timer::ID EventLoop::runEvery(double time, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now(), time);
}
void EventLoop::cancelTimer(Timer::ID id) {
    timerQueue_->cancel(id);
}

void EventLoop::runInLoop(Function func) {
    if(isInLoopThread()) {
        func();
    } else {
        queueInLoop(func);
        wakeup();
    }
}
void EventLoop::queueInLoop(Function func) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push_back(func);
    }
}

/* 通过eventfd来唤醒poll */
void EventLoop::wakeup() {
    int ret = eventfd_write(wakeupFd_, 1);
    if(ret == -1) {
        LOG_ERROR("Failed to write eventfd({})", strerror(errno));
    }
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
int EventLoop::numOfEvents() { numOfEvents_ = activeEvents_.size(); return numOfEvents_; }

std::string EventLoop::tidToStr() const {
    std::stringstream ss;
    ss << tid_;
    std::string tidStr = ss.str();
    tidStr = tidStr.substr(tidStr.length() - 4, 4);
    return tidStr;
}