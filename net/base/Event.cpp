#include "net/base/Event.h"

/* Local headers */
#include "net/base/Looper.h"
#include "logger/Logger.h"
#include "utils/ErrorInfo.h"

using esynet::Event;
using esynet::Looper;

Event::Event(Looper& looper, int fd): looper_(looper), fd_(fd) {}

void Event::handle() {
    if(happenedEvents_ & kCloseEvent) {
        LOG_DEBUG("handle close event(fd: {})", fd_);
        if(closeCallback_) closeCallback_();
    }
    if(happenedEvents_ & kReadEvent) {
        LOG_DEBUG("handle read event(fd: {})", fd_);
        if(readCallback_) readCallback_();
    }
    if(happenedEvents_ & kWriteEvent) {
        LOG_DEBUG("handle write event(fd: {})", fd_);
        if(writeCallback_) writeCallback_();
    }
    if(happenedEvents_ & kErrorEvent) {
        LOG_DEBUG("handle error event(fd: {})", fd_);
        if(errorCallback_) errorCallback_();
    }
}

void Event::setCloseCallback(Callback callback) { closeCallback_ = std::move(callback); }
void Event::setReadCallback(Callback callback)  { readCallback_  = std::move(callback); }
void Event::setWriteCallback(Callback callback) { writeCallback_ = std::move(callback); }
void Event::setErrorCallback(Callback callback) { errorCallback_ = std::move(callback); }

int      Event::fd()            const { return fd_; }
short    Event::listenedEvent() const { return listenedEvents_; }
bool     Event::writable()      const { return happenedEvents_ & kWriteEvent; }
bool     Event::readable()      const { return happenedEvents_ & kReadEvent; }
Looper*  Event::looper()        const { return &looper_; }
int      Event::index()         const { return indexInPoll_; }
void Event::setHappenedEvent(int event) { happenedEvents_ = event; }
void Event::setIndex(int index) { indexInPoll_ = index; }

void Event::enableRead() {
    if(listenedEvents_ == kNoneEvent) listenedEvents_ = 0;
    listenedEvents_ |= kReadEvent;
    update();
}
void Event::enableWrite() {
    if(listenedEvents_ == kNoneEvent) listenedEvents_ = 0;
    listenedEvents_ |= kWriteEvent;
    update();
}
void Event::disableWrite() {
    listenedEvents_ &= ~kWriteEvent;
    if(listenedEvents_ == 0) listenedEvents_ = kNoneEvent;
    update();
}
void Event::disableRead() {
    listenedEvents_ &= ~kReadEvent;
    if(listenedEvents_ == 0) listenedEvents_ = kNoneEvent;
    update();
}
void Event::cancel() {
    listenedEvents_ = kNoneEvent;
    looper_.removeEvent(*this);
}
void Event::update() {
    looper_.updateEvent(*this);
}