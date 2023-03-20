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
    using ReactorPtr   = std::unique_ptr<Looper>;
    using InitCallback = std::function<void(Looper&)>;

public:
    ReactorThreadPoll(Looper&);
    ~ReactorThreadPoll();

    void start();
    void stop();

    auto getNext()     -> Looper*;   /* 按顺序获取下一个 */
    auto getLightest() -> Looper*;   /* 获取负载最轻的一个 */
    auto getAllReactors() -> std::vector<Looper*>;

    void setInitCallback(InitCallback);
    void setThreadNum(size_t);

private:
    std::mutex mutex_;
    bool   start_     {false};
    int    index_     {0};
    size_t threadNum_ {0};
    Looper& mainReactor_;
    InitCallback initCb_;
    std::vector<ReactorPtr> reactors_;
    std::vector<std::thread> threads_;
};

}