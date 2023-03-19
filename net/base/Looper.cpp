#include "net/base/Looper.h"

/* Local headers */
#include "net/base/Event.h"
#include "logger/Logger.h"
#include "net/poller/PollPoller.h"
#include "net/poller/EpollPoller.h"
#include "net/timer/TimerQueue.h"
#include "utils/Timestamp.h"
#include "utils/ErrorInfo.h"

/* Linux headers */
#include <sys/eventfd.h>

using esynet::Looper;
using esynet::poller::EpollPoller;
using esynet::poller::PollPoller;
using esynet::utils::Timestamp;
using esynet::timer::Timer;
using esynet::Logger;

/* 每个线程至多有一个 Looper */
thread_local Looper* t_reactorInCurThread = nullptr;
/* 超时时间 */
const int Looper::kPollTimeMs = 3000;

int createEventFd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd == -1) {
        LOG_ERROR("Failed to create eventfd({})", errnoStr(errno));
    }
    return fd;
}

Looper::Looper(bool useEpoll):
            stop_(false),
            numOfEvents_(0),
            isLooping_(false),
            wakeupFd_(createEventFd()),
            wakeupEvent_(*this, wakeupFd_),
            tid_(std::this_thread::get_id()) {
    if(t_reactorInCurThread) {
        LOG_FATAL("Another Looper({:p}) exists in this thread({})",
                    static_cast<void*>(t_reactorInCurThread), tidToStr(tid_));
    } else {
        t_reactorInCurThread = this;
        LOG_DEBUG("Looper({:p}) created in thread {}",
                    static_cast<void*>(this), tidToStr(tid_));
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
            LOG_ERROR("Failed to read eventfd({})", errnoStr(errno));
        }
        LOG_DEBUG("Wake up Looper({:p})", static_cast<void*>(this));
    });
    wakeupEvent_.enableRead();
}

Looper::~Looper() {
    if(isLooping_) {
        LOG_FATAL("Deconstruct Looper({:p}) while is looping",
                    static_cast<void*>(t_reactorInCurThread));
    }
    t_reactorInCurThread = nullptr;
    removeEvent(wakeupEvent_);
}

void Looper::start() {
    assert();

    stop_ = false;
    isLooping_ = true;
    LOG_DEBUG("Looper({:p}) start looping", static_cast<void*>(this));
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
    LOG_DEBUG("Looper({:p}) stop looping", static_cast<void*>(this));
    isLooping_ = false;
}
void Looper::stop() { stop_ = true; wakeup(); }
Timestamp Looper::lastPollTime() {
    std::unique_lock<std::mutex> lock(mutex_);
    return lastPollTime_;
}

/* 该部分的线程安全由TimerQueue保证 */
Timer::ID Looper::runAt(Timestamp timePoint, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, timePoint, 0.0);
}
Timer::ID Looper::runAfter(double delay, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now() + delay, 0.0);
}
Timer::ID Looper::runEvery(double interval, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now(), interval);
}
void Looper::cancelTimer(Timer::ID id) {
    timerQueue_->cancel(id);
}

void Looper::run(Function func) {
    if(isInLoopThread()) {
        func();
    } else {
        queue(func);
        wakeup();
    }
}
void Looper::queue(Function func) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push_back(func);
    }
}

/* 通过eventfd来唤醒poll */
void Looper::wakeup() {
    if(eventfd_write(wakeupFd_, 1) == -1) {
        LOG_ERROR("Failed to write eventfd({})", errnoStr(errno));
    }
}

/* 将Event委托的update转发给poller */
void Looper::updateEvent(Event& event) {
    if(event.looper() != this) {
        LOG_FATAL("Event({}) doesn't belong to loop({:p})",
            event.fd(), static_cast<void*>(this));
    }
    LOG_DEBUG("Update event (fd: {})", event.fd());
    poller_->updateEvent(event);
}
void Looper::removeEvent(Event& event) {
    if(event.looper() != this) {
        LOG_FATAL("Event({}) doesn't belong to loop({:p})",
            event.fd(), static_cast<void*>(this));
    }
    LOG_DEBUG("Remove event (fd: {})", event.fd());
    poller_->removeEvent(event);
}

bool Looper::isInLoopThread() const {
    return tid_ == std::this_thread::get_id();
}
void Looper::assert() const {
    if(!(tid_ == std::this_thread::get_id())) {
        LOG_FATAL("Looper({}) called by another thread({})", tidToStr(tid_), tidToStr(std::this_thread::get_id()));
    }
}
bool Looper::isLooping() const { return isLooping_; }
int Looper::numOfEvents() { numOfEvents_ = activeEvents_.size(); return numOfEvents_; }

std::string Looper::tidToStr(std::thread::id tid) const {
    std::stringstream ss;
    ss << tid;
    std::string tidStr = ss.str();
    tidStr = tidStr.substr(tidStr.length() - 4, 4);
    return tidStr;
}