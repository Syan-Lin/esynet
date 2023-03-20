#pragma once

/* Standard headers */
#include <functional>
#include <atomic>

/* Local headers */
#include "utils/Timestamp.h"

namespace esynet::timer {

using utils::Timestamp;

class Timer {
public:
    using Callback = std::function<void()>;
    using ID = int64_t;

public:
    Timer(Callback, Timestamp expiration, double interval);

    void run();
    void restart();

    auto expiration() const -> Timestamp;
    bool repeat() const;
    ID id() const;

private:
    static std::atomic<ID> idCounter_;
    static ID nextId();

private:
    const ID id_;
    const bool repeat_;
    Timestamp expiration_;
    const double interval_; /* 单位：毫秒 */
    const Callback callback_;

};

} /* namespace esynet::timer */