#include "net/thread/EventLoopThreadPoll.h"
#include "logger/Logger.h"
#include <thread>

using esynet::EventLoopThreadPoll;
using esynet::EventLoop;

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop& loop)
    : baseLoop_(loop), start_(false), index_(0), threadNum_(0) {}
EventLoopThreadPoll::~EventLoopThreadPoll() { stop(); }

void EventLoopThreadPoll::start() {
    if(start_ || threadNum_ == 0) return;
    if(!baseLoop_.isInLoopThread()) {
        LOG_FATAL("Try start thread poll in another thread(baseLoop: {:p})",
                    static_cast<void*>(&baseLoop_));
    }
    for(size_t i = 0; i < threadNum_; i++) {
        threads_.emplace_back(std::thread([this] {
            /* 线程任务 */
            {
                std::unique_lock<std::mutex> lock(mutex_);
                loops_.emplace_back(std::make_unique<EventLoop>());
            }
            if(initCb_) {
                initCb_(*loops_.back());
            }
            loops_.back()->loop();
        }));
    }
    start_ = true;
}
void EventLoopThreadPoll::stop() {
    if(!start_) return;
    for(int i = 0; i < loops_.size(); i++) {
        loops_[i]->stop();
    }
    for(int i = 0; i < threads_.size(); i++) {
        threads_[i].join();
    }
    loops_.clear();
    threads_.clear();
    start_ = false;
}

EventLoop* EventLoopThreadPoll::getNext() {
    if(!start_ || loops_.empty()) return &baseLoop_;
    if(index_ == loops_.size()) index_ = 0;
    return loops_[index_++].get();
}
/* 线程数量常数级，没有必要优化 */
EventLoop* EventLoopThreadPoll::getLightest() {
    if(!start_ || loops_.empty()) return &baseLoop_;
    int min = INT_MAX, index = 0;
    for(int i = 0; i < loops_.size(); i++) {
        int num = loops_[i]->numOfEvents();
        if(num < min) {
            min = num;
            index = i;
        }
    }
    return loops_[index].get();
}
std::vector<EventLoop*> EventLoopThreadPoll::getAllLoops() {
    if(!start_ || loops_.empty()) return {&baseLoop_};
    std::vector<EventLoop*> loops;
    for(int i = 0; i < loops_.size(); i++) {
        loops.emplace_back(loops_[i].get());
    }
    return loops;
}

void EventLoopThreadPoll::setInitCallback(InitCallback cb) {
    initCb_ = cb;
}
void EventLoopThreadPoll::setThreadNum(size_t num) {
    threadNum_ = num;
}