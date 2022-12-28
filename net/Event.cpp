#include "net/Event.h"
#include "net/EventLoop.h"

Event::Event(EventLoop& loop, int fd)
        : loop_(loop), fd_(fd), listenedEvents_(-1),
          happenedEvents_(0), indexInPoll_(-1) {}

void Event::handle() {
    if(happenedEvents_ & kReadEvent) {
        if(readCallback_) readCallback_();
    }
    if(happenedEvents_ & kWriteEvent) {
        if(writeCallback_) writeCallback_();
    }
    if(happenedEvents_ & kErrorEvent) {
        if(errorCallback_) errorCallback_();
    }
}

inline void Event::setReadCallback(Callback callback) {
    readCallback_ = callback;
}
inline void Event::setWriteCallback(Callback callback) {
    writeCallback_ = callback;
}
inline void Event::setErrorCallback(Callback callback) {
    errorCallback_ = callback;
}

inline short Event::fd() const { return fd_; }
inline short Event::listenedEvent() const { return listenedEvents_; }
inline void Event::setHappenedEvent(int event) { happenedEvents_ = event; }

void Event::enableReading() {
    listenedEvents_ |= kReadEvent;
    update();
}
void Event::enableWriting() {
    listenedEvents_ |= kWriteEvent;
    update();
}
void Event::disableWriting() {
    listenedEvents_ &= ~kWriteEvent;
    update();
}
void Event::disableReading() {
    listenedEvents_ &= ~kReadEvent;
    update();
}
void Event::disableAll() {
    listenedEvents_ = kNoneEvent;
    update();
}

inline int Event::index() const { return indexInPoll_; }
inline void Event::setIndex(int index) { indexInPoll_ = index; }
EventLoop* Event::ownerLoop() const { return &loop_; }

void Event::update() {
    loop_.updateEvent(*this);
}