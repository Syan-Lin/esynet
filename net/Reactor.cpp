#include "net/Reactor.h"

/* Local headers */
#include "net/Event.h"
#include "logger/Logger.h"
#include "net/poller/PollPoller.h"
#include "net/poller/EpollPoller.h"
#include "net/timer/TimerQueue.h"
#include "utils/Timestamp.h"
#include "utils/ErrorInfo.h"

/* Linux headers */
#include <sys/eventfd.h>

using esynet::Reactor;
using esynet::poller::EpollPoller;
using esynet::poller::PollPoller;
using esynet::utils::Timestamp;
using esynet::timer::Timer;
using esynet::Logger;

/* 每个线程至多有一个 Reactor */
thread_local Reactor* t_reactorInCurThread = nullptr;
/* 超时时间 */
const int Reactor::kPollTimeMs = 3000;

Reactor* Reactor::getReactorOfCurThread() {
    return t_reactorInCurThread;
}

int createEventFd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd == -1) {
        LOG_ERROR("Failed to create eventfd({})", errnoStr(errno));
    }
    return fd;
}

Reactor::Reactor(bool useEpoll)
        : stop_(false),
          numOfEvents_(0),
          isLooping_(false),
          wakeupFd_(createEventFd()),
          wakeupEvent_(*this, wakeupFd_),
          tid_(std::this_thread::get_id()) {
    if(t_reactorInCurThread) {
        LOG_FATAL("Another Reactor({:p}) exists in this thread({})",
                    static_cast<void*>(t_reactorInCurThread), tidToStr());
    } else {
        t_reactorInCurThread = this;
        LOG_DEBUG("Reactor({:p}) created in thread {}",
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
            LOG_ERROR("Failed to read eventfd({})", errnoStr(errno));
        }
        LOG_DEBUG("Wake up Reactor({:p})", static_cast<void*>(this));
    });
    wakeupEvent_.enableReading();
}

Reactor::~Reactor() {
    if(isLooping_) {
        LOG_FATAL("Deconstruct Reactor({:p}) while is looping",
                    static_cast<void*>(t_reactorInCurThread));
    }
    t_reactorInCurThread = nullptr;
    removeEvent(wakeupEvent_);
}

void Reactor::start() {
    if(!isInLoopThread()) {
        LOG_FATAL("Try to do loop({:p}) in another thread", static_cast<void*>(this));
    }
    stop_ = false;
    isLooping_ = true;
    LOG_DEBUG("Reactor({:p}) start looping", static_cast<void*>(this));
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
    LOG_DEBUG("Reactor({:p}) stop looping", static_cast<void*>(this));
    isLooping_ = false;
}
void Reactor::stop() { stop_ = true; wakeup(); }
Timestamp Reactor::lastPollTime() {
    std::unique_lock<std::mutex> lock(mutex_);
    return lastPollTime_;
}

/* 该部分的线程安全由TimerQueue保证 */
Timer::ID Reactor::runAt(Timestamp timePoint, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, timePoint, 0.0);
}
Timer::ID Reactor::runAfter(double delay, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now() + delay, 0.0);
}
Timer::ID Reactor::runEvery(double interval, Timer::Callback callback) {
    return timerQueue_->addTimer(callback, Timestamp::now(), interval);
}
void Reactor::cancelTimer(Timer::ID id) {
    timerQueue_->cancel(id);
}

void Reactor::run(Function func) {
    if(isInLoopThread()) {
        func();
    } else {
        queue(func);
        wakeup();
    }
}
void Reactor::queue(Function func) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push_back(func);
    }
}

/* 通过eventfd来唤醒poll */
void Reactor::wakeup() {
    int ret = eventfd_write(wakeupFd_, 1);
    if(ret == -1) {
        LOG_ERROR("Failed to write eventfd({})", errnoStr(errno));
    }
}

/* 将Event委托的update转发给poller */
void Reactor::updateEvent(Event& event) {
    if(event.owner() != this) {
        LOG_FATAL("Event({}) doesn't belong to loop({:p})",
            event.fd(), static_cast<void*>(this));
    }
    poller_->updateEvent(event);
}
void Reactor::removeEvent(Event& event) {
    if(event.owner() != this) {
        LOG_FATAL("Event({}) doesn't belong to loop({:p})",
            event.fd(), static_cast<void*>(this));
    }
    poller_->removeEvent(event);
}

bool Reactor::isInLoopThread() const {
    return tid_ == std::this_thread::get_id();
}
bool Reactor::isLooping() const { return isLooping_; }
int Reactor::numOfEvents() { numOfEvents_ = activeEvents_.size(); return numOfEvents_; }

std::string Reactor::tidToStr() const {
    std::stringstream ss;
    ss << tid_;
    std::string tidStr = ss.str();
    tidStr = tidStr.substr(tidStr.length() - 4, 4);
    return tidStr;
}