#pragma once

/* Standard headers */
#include <functional>
#include <thread>

/* Local headers */
#include "utils/Timestamp.h"

namespace esynet::timer {

using utils::Timestamp;

class Timer {
public:
    using Callback = std::function<void()>;
    using ID = int64_t;

public:
    Timer(Callback, Timestamp, double);

    void run();
    void restart();

    Timestamp expiration() const;
    bool repeat() const;
    ID id() const;

private:
    static std::atomic<ID> idCounter_;
    static ID nextId();

private:
    const ID id_;
    const bool repeat_;
    const Callback callback_;
    const double interval_;     /* 单位：毫秒 */
    Timestamp expiration_;

};

} /* namespace esynet::timer */