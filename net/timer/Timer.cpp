#include "net/timer/Timer.h"

std::atomic<Timer::ID> Timer::idCounter_(0);
Timer::ID Timer::nextId() { return idCounter_++; }

Timer::Timer(Callback callback, Timestamp expiration, double interval)
        : id_(nextId()), repeat_(interval > 0.0), callback_(callback),
          interval_(interval), expiration_(expiration) {}

void Timer::run() {
    callback_();
}
void Timer::restart() {
    if(repeat_) {
        expiration_ = Timestamp::now() + interval_;
    } else {
        expiration_ = Timestamp();
    }
}

Timestamp Timer::expiration() const { return expiration_; }
bool Timer::repeat() const { return repeat_; }
Timer::ID Timer::id() const { return id_; }