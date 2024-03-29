#include "net/thread/ReactorThreadPoll.h"
#include "logger/Logger.h"
#include <thread>

using esynet::ReactorThreadPoll;
using esynet::Looper;

ReactorThreadPoll::ReactorThreadPoll(Looper& looper): mainReactor_(looper) {}
ReactorThreadPoll::~ReactorThreadPoll() { stop(); }

void ReactorThreadPoll::start() {
    if(start_ || threadNum_ == 0) return;
    mainReactor_.assert();
    for(size_t i = 0; i < threadNum_; i++) {
        threads_.emplace_back(std::thread([this] {
            /* 线程任务 */
            {
                std::unique_lock<std::mutex> lock(mutex_);
                reactors_.emplace_back(std::make_unique<Looper>());
            }
            if(initCb_) {
                initCb_(*reactors_.back());
            }
            reactors_.back()->start();
        }));
    }
    start_ = true;
}
void ReactorThreadPoll::stop() {
    if(!start_) return;
    for(int i = 0; i < reactors_.size(); i++) {
        reactors_[i]->stop();
    }
    for(int i = 0; i < threads_.size(); i++) {
        threads_[i].join();
    }
    reactors_.clear();
    threads_.clear();
    start_ = false;
}

Looper* ReactorThreadPoll::getNext() {
    if(!start_ || reactors_.empty()) return &mainReactor_;
    if(index_ == reactors_.size()) index_ = 0;
    return reactors_[index_++].get();
}
/* 线程数量常数级，没有必要优化 */
Looper* ReactorThreadPoll::getLightest() {
    if(!start_ || reactors_.empty()) return &mainReactor_;
    int min = INT_MAX, index = 0;
    for(int i = 0; i < reactors_.size(); i++) {
        int num = reactors_[i]->numOfEvents();
        if(num < min) {
            min = num;
            index = i;
        }
    }
    return reactors_[index].get();
}
std::vector<Looper*> ReactorThreadPoll::getAllReactors() {
    if(!start_ || reactors_.empty()) return {&mainReactor_};
    std::vector<Looper*> reactors;
    for(int i = 0; i < reactors_.size(); i++) {
        reactors.emplace_back(reactors_[i].get());
    }
    return reactors;
}

void ReactorThreadPoll::setInitCallback(InitCallback cb) {
    initCb_ = std::move(cb);
}
void ReactorThreadPoll::setThreadNum(size_t num) {
    threadNum_ = num;
}