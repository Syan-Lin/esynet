#include "net/thread/EventLoopThreadPoll.h"
#include "logger/Logger.h"
#include <thread>

using esynet::EventLoopThreadPoll;
using esynet::EventLoop;

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop& loop)
    : baseLoop_(loop), start_(false), index_(0) {}
EventLoopThreadPoll::~EventLoopThreadPoll() { stop(); }

void EventLoopThreadPoll::start(size_t num) {
    if(start_) return;
    if(!baseLoop_.isInLoopThread()) {
        LOG_FATAL("Try start thread poll in another thread(baseLoop: {:p})",
                    static_cast<void*>(&baseLoop_));
    }
    for(size_t i = 0; i < num; i++) {
        threads_.emplace_back(std::thread([this] {
            /* 线程任务 */
            EventLoop* loop;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                loops_.emplace_back(std::make_unique<EventLoop>());
                loop = loops_.back().get();
            }
            if(initCall_) {
                initCall_(loop);
            }
            loop->loop();
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
    if(!start_ || loops_.size() == 0) return nullptr;
    if(index_ == loops_.size()) index_ = 0;
    return loops_[index_++].get();
}
EventLoop* EventLoopThreadPoll::getLightest() {
    if(!start_ || loops_.size() == 0) return nullptr;
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
    std::vector<EventLoop*> loops;
    for(int i = 0; i < loops_.size(); i++) {
        loops.emplace_back(loops_[i].get());
    }
    return loops;
}

void EventLoopThreadPoll::setInitCallback(InitCallback cb) {
    initCall_ = cb;
}
