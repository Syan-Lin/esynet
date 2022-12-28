#include "net/poller/EpollPoller.h"
#include "logger/Logger.h"

const int EpollPoller::kInitEventListSize = 16;

EpollPoller::EpollPoller(EventLoop& loop)
    : Poller(loop),
      epollFd_(epoll_create1(EPOLL_CLOEXEC)),
      epollEvents_(kInitEventListSize) {
    if(epollFd_ < 0) {
        LOG_FATAL("epoll_create1 error");
    }
}

EpollPoller::~EpollPoller() { close(epollFd_); }

Timestamp EpollPoller::poll(EventList& activeEvents, int timeoutMs) {
    int numEvents = epoll_wait(epollFd_, &*epollEvents_.begin(), epollEvents_.size(), timeoutMs);
    Timestamp pollTime(Timestamp::now());
    if(numEvents > 0) {
        LOG_DEBUG("{} events happened", numEvents);
        fillActiveEvents(numEvents, activeEvents);
    } else if(numEvents == 0) {
        LOG_DEBUG("Nothing happened");
    } else {
        LOG_ERROR("Poller poll error");
    }
    return pollTime;
}

void EpollPoller::updateEvent(Event& event) {
    LOG_DEBUG("Update event(fd: {}, events: {})", event.fd(), event.listenedEvent());
    if(event.index() < 0) {
        /* 添加至列表 */
        if(events_.find(event.fd()) != events_.end()) {
            LOG_ERROR("Event(fd: {}) already exists", event.fd());
        }
        epollUpdate(EPOLL_CTL_ADD, event);
    } else {
        /* 更新列表 */
        if(events_.find(event.fd()) == events_.end()) {
            LOG_ERROR("Event(fd: {}) not exists", event.fd());
        } else if(events_[event.fd()] != &event) {
            LOG_ERROR("Event(fd: {}) not the same", event.fd());
        }
        epollUpdate(EPOLL_CTL_MOD, event);
    }
}

void EpollPoller::removeEvent(Event& event) {
    LOG_DEBUG("Remove event(fd: {})", event.fd());
    if(event.index() < 0) {
        return;
    } else {
        if(events_.find(event.fd()) == events_.end()) {
            LOG_ERROR("Event(fd: {}) not exists", event.fd());
        } else if(events_[event.fd()] != &event) {
            LOG_ERROR("Event(fd: {}) not the same", event.fd());
        }
        epollUpdate(EPOLL_CTL_DEL, event);
    }
}

void EpollPoller::epollUpdate(int operation, Event& event) {
    EpollEvent epollEvent;
    epollEvent.events = event.listenedEvent();
    epollEvent.data.fd = event.fd();
    if(epoll_ctl(epollFd_, operation, event.fd(), &epollEvent) < 0) {
        LOG_ERROR("epoll_ctl({}) error", operation);
    }
    if(operation == EPOLL_CTL_ADD) {
        epollEvents_.push_back(epollEvent);
        event.setIndex(epollEvents_.size() - 1);
        events_[event.fd()] = &event;
    } else if(operation == EPOLL_CTL_DEL) {
        events_.erase(event.fd());
        int index = event.index();
        if(index >= epollEvents_.size()) {
            LOG_ERROR("Event(fd: {}) index out of range", event.fd());
        }
        epollEvents_.erase(epollEvents_.begin() + index);
        event.setIndex(-event.fd() - 1);
    }
}

void EpollPoller::fillActiveEvents(int numEvents, EventList& activeEvents) const {
    for(int i = 0; i < numEvents; ++i) {
        Event* event = events_.at(epollEvents_[i].data.fd);
        event->setHappenedEvent(epollEvents_[i].events);
        activeEvents.push_back(event);
    }
}