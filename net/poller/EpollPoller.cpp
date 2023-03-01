#include "net/poller/EpollPoller.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/Event.h"
#include "utils/Timestamp.h"
#include "utils/ErrorInfo.h"

using esynet::poller::EpollPoller;
using esynet::utils::Timestamp;

const int EpollPoller::kInitEventListSize = 16;

EpollPoller::EpollPoller(Reactor& reactor):
                    Poller(reactor),
                    epollFd_(epoll_create1(EPOLL_CLOEXEC)),
                    epollEvents_(kInitEventListSize) {
    if(epollFd_ < 0) {
        LOG_FATAL("epoll_create1 error(err: {})", errnoStr(errno));
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
        LOG_ERROR("epoll_wait error: {}", errnoStr(errno));
    }
    return pollTime;
}

void EpollPoller::updateEvent(Event& event) {
    LOG_DEBUG("Update event(fd: {}, events: {})", event.fd(), event.listenedEvent());
    if(event.index() < 0) {
        if(event.listenedEvent() < 0) return;
        /* 添加至列表 */
        if(events_.find(event.fd()) != events_.end()) {
            LOG_ERROR("Event(fd: {}) already exists", event.fd());
        }
        if(event.listenedEvent() < 0) return;
        epollUpdate(EPOLL_CTL_ADD, event);
    } else {
        /* 更新列表 */
        if(events_.find(event.fd()) == events_.end()) {
            LOG_ERROR("Event(fd: {}) not exists", event.fd());
        } else if(events_[event.fd()] != &event) {
            LOG_ERROR("Event(fd: {}) not the same", event.fd());
        }
        if(event.listenedEvent() < 0) {
            removeEvent(event);
        } else {
            epollUpdate(EPOLL_CTL_MOD, event);
        }
    }
}

void EpollPoller::removeEvent(Event& event) {
    epollUpdate(EPOLL_CTL_DEL, event);

    events_.erase(event.fd());
    int index = event.index();
    if(index >= epollEvents_.size() || index < 0) {
        LOG_ERROR("Event(fd: {}) index out of range", event.fd());
    }
    if(index == epollEvents_.size() - 1) {
        epollEvents_.pop_back();
    } else {
        int lastFd = epollEvents_.back().data.fd;
        std::iter_swap(epollEvents_.begin() + index, epollEvents_.end() - 1);
        epollEvents_.pop_back();
        events_[lastFd]->setIndex(index);
    }
    event.setIndex(-event.fd() - 1);
}

void EpollPoller::epollUpdate(int operation, Event& event) {
    EpollEvent epollEvent;
    epollEvent.events = event.listenedEvent();
    epollEvent.data.fd = event.fd();
    if(epoll_ctl(epollFd_, operation, event.fd(), &epollEvent) < 0) {
        LOG_ERROR("epoll_ctl(op: {}, fd: {}, errno: {}) error",
                    operation, event.fd(), errnoStr(errno));
        event.setHappenedEvent(errno);
        event.handle();
    }
    if(operation == EPOLL_CTL_ADD) {
        epollEvents_.push_back(epollEvent);
        event.setIndex(epollEvents_.size() - 1);
        events_[event.fd()] = &event;
    }
}

void EpollPoller::fillActiveEvents(int numEvents, EventList& activeEvents) const {
    for(int i = 0; i < numEvents; ++i) {
        Event* event = events_.at(epollEvents_[i].data.fd);
        event->setHappenedEvent(epollEvents_[i].events);
        activeEvents.push_back(event);
    }
}