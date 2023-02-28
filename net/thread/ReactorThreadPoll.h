#pragma once

/* Standard headers */
#include <queue>
#include <memory>
#include <thread>

/* Local headers */
#include "net/Reactor.h"

namespace esynet {

class ReactorThreadPoll {
private:
    using ReactorPtr = std::unique_ptr<Reactor>;
    using InitCallback = std::function<void(Reactor&)>;

public:
    ReactorThreadPoll(Reactor&);
    ~ReactorThreadPoll();

    void start();
    void stop();

    Reactor* getNext();       /* 按顺序获取下一个 */
    Reactor* getLightest();   /* 获取任务最轻的一个 */
    std::vector<Reactor*> getAllReactors();

    void setInitCallback(InitCallback);
    void setThreadNum(size_t);

private:
    Reactor& mainReactor_;
    std::vector<ReactorPtr> reactors_;
    std::vector<std::thread> threads_;
    InitCallback initCb_;
    std::mutex mutex_;
    bool start_;
    int index_;
    size_t threadNum_;
};

}