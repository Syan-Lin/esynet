#include "net/Event.h"

/* Local headers */
#include "net/Reactor.h"

using esynet::Event;
using esynet::Reactor;

Event::Event(Reactor& loop, int fd)
        : reactor_(loop), fd_(fd), listenedEvents_(-1),
          happenedEvents_(0), indexInPoll_(-1) {}

void Event::handle() {
    if(happenedEvents_ & kCloseEvent) {
        if(closeCallback_) closeCallback_();
    }
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

void Event::setCloseCallback(Callback callback) {
    closeCallback_ = callback;
}
void Event::setReadCallback(Callback callback) {
    readCallback_ = callback;
}
void Event::setWriteCallback(Callback callback) {
    writeCallback_ = callback;
}
void Event::setErrorCallback(Callback callback) {
    errorCallback_ = callback;
}

int Event::fd() const { return fd_; }
short Event::listenedEvent() const { return listenedEvents_; }
bool Event::writable() const { return happenedEvents_ & kWriteEvent; }
bool Event::readable() const { return happenedEvents_ & kReadEvent; }
void Event::setHappenedEvent(int event) { happenedEvents_ = event; }

void Event::enableReading() {
    if(listenedEvents_ == kNoneEvent) listenedEvents_ = 0;
    listenedEvents_ |= kReadEvent;
    update();
}
void Event::enableWriting() {
    if(listenedEvents_ == kNoneEvent) listenedEvents_ = 0;
    listenedEvents_ |= kWriteEvent;
    update();
}
void Event::disableWriting() {
    listenedEvents_ &= ~kWriteEvent;
    if(listenedEvents_ == 0) listenedEvents_ = kNoneEvent;
    update();
}
void Event::disableReading() {
    listenedEvents_ &= ~kReadEvent;
    if(listenedEvents_ == 0) listenedEvents_ = kNoneEvent;
    update();
}
void Event::disableAll() {
    listenedEvents_ = kNoneEvent;
    update();
}

int Event::index() const { return indexInPoll_; }
void Event::setIndex(int index) { indexInPoll_ = index; }
Reactor* Event::owner() const { return &reactor_; }

void Event::update() {
    reactor_.updateEvent(*this);
}