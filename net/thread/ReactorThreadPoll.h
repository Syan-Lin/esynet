#pragma once

/* Standard headers */
#include <queue>
#include <memory>
#include <thread>

/* Local headers */
#include "net/base/Looper.h"

namespace esynet {

class ReactorThreadPoll {
private:
    using ReactorPtr = std::unique_ptr<Looper>;
    using InitCallback = std::function<void(Looper&)>;

public:
    ReactorThreadPoll(Looper&);
    ~ReactorThreadPoll();

    void start();
    void stop();

    Looper* getNext();       /* 按顺序获取下一个 */
    Looper* getLightest();   /* 获取负载最轻的一个 */
    std::vector<Looper*> getAllReactors();

    void setInitCallback(InitCallback);
    void setThreadNum(size_t);

private:
    Looper& mainReactor_;
    std::vector<ReactorPtr> reactors_;
    std::vector<std::thread> threads_;
    InitCallback initCb_;
    std::mutex mutex_;
    bool start_;
    int index_;
    size_t threadNum_;
};

}