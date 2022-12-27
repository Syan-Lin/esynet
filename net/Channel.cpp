#include "net/Channel.h"
#include "net/EventLoop.h"

Channel::Channel(EventLoop& loop, int fd)
        : loop_(loop), fd_(fd), events_(-1),
          revents_(0), index_(-1) {}

void Channel::handleEvent() {
    if(revents_ & kReadEvent) {
        if(readCallback_) readCallback_();
    }
    if(revents_ & kWriteEvent) {
        if(writeCallback_) writeCallback_();
    }
    if(revents_ & kErrorEvent) {
        if(errorCallback_) errorCallback_();
    }
}

inline void Channel::setReadCallback(Callback callback) {
    readCallback_ = callback;
}
inline void Channel::setWriteCallback(Callback callback) {
    writeCallback_ = callback;
}
inline void Channel::setErrorCallback(Callback callback) {
    errorCallback_ = callback;
}

inline short Channel::fd() const { return fd_; }
inline short Channel::events() const { return events_; }
inline void Channel::setRevents(int revents) { revents_ = revents; }

void Channel::enableReading() {
    events_ |= kReadEvent;
    update();
}
void Channel::enableWriting() {
    events_ |= kWriteEvent;
    update();
}
void Channel::disableWriting() {
    events_ &= ~kWriteEvent;
    update();
}
void Channel::disableReading() {
    events_ &= ~kReadEvent;
    update();
}
void Channel::disableAll() {
    events_ = kNoneEvent;
    update();
}

inline int Channel::index() const { return index_; }
inline void Channel::setIndex(int index) { index_ = index; }
EventLoop* Channel::ownerLoop() const { return &loop_; }

void Channel::update() {
    loop_.updateChannel(*this);
}