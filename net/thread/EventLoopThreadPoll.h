#pragma once

/* Standard headers */
#include <queue>
#include <memory>
#include <thread>

/* Local headers */
#include "net/EventLoop.h"

namespace esynet {

/* 非线程安全，不要跨线程调用，该线程池应该只属于一个主线程 */
class EventLoopThreadPoll {
private:
    using EventLoopPtr = std::unique_ptr<EventLoop>;
    using InitCallback = std::function<void(EventLoop*)>;

public:
    EventLoopThreadPoll(EventLoop&);
    ~EventLoopThreadPoll();

    void start(size_t numOfThreads);
    void stop();

    EventLoop* getNext();       /* 按顺序获取下一个 */
    EventLoop* getLightest();   /* 获取任务最清闲的一个 */
    std::vector<EventLoop*> getAllLoops();

    void setInitCallback(InitCallback);

private:
    EventLoop& baseLoop_;
    std::vector<EventLoopPtr> loops_;
    std::vector<std::thread> threads_;
    InitCallback initCall_;
    std::mutex mutex_;
    bool start_;
    int index_;
};

}