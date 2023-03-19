#include "net/poller/PollPoller.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/Event.h"
#include "utils/ErrorInfo.h"

using esynet::poller::PollPoller;
using esynet::utils::Timestamp;

PollPoller::PollPoller(Looper& looper) : Poller(looper) {}

Timestamp PollPoller::poll(EventList& activeEvents, int timeoutMs) {
    int numEvents = ::poll(&*pollFds_.begin(), pollFds_.size(), timeoutMs);
    Timestamp pollTime(Timestamp::now());
    if(numEvents > 0) {
        LOG_DEBUG("{} events happened", numEvents);
        fillActiveEvents(numEvents, activeEvents);
    } else if(numEvents == 0) {
        LOG_DEBUG("Nothing happened");
    } else {
        LOG_ERROR("poll error: {}", errnoStr(errno));
    }
    return pollTime;
}

void PollPoller::updateEvent(Event& event) {
    LOG_DEBUG("Update event(fd: {}, events: {})", event.fd(), event.listenedEvent());
    if(event.index() < 0) {
        if(event.listenedEvent() < 0) return;
        /* 添加至列表 */
        if(events_.find(event.fd()) != events_.end()) {
            LOG_ERROR("Event(fd: {}) already exists", event.fd());
        }
        PollFd pfd;
        pfd.fd = event.fd();
        pfd.events = event.listenedEvent();
        pfd.revents = 0;
        pollFds_.push_back(pfd);
        event.setIndex(pollFds_.size() - 1);
        events_[pfd.fd] = &event;
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
            PollFd& pfd = pollFds_.at(event.index());
            pfd.events = event.listenedEvent();
            pfd.revents = 0;
        }
    }
}

void PollPoller::removeEvent(Event& event) {
    events_.erase(event.fd());
    int index = event.index();
    if(index >= pollFds_.size() || index < 0) {
        LOG_ERROR("Event(fd: {}) index out of range", event.fd());
    } else if(index == pollFds_.size() - 1) {
        pollFds_.pop_back();
    } else {
        int lastFd = pollFds_.back().fd;
        std::iter_swap(pollFds_.begin() + index, pollFds_.end() - 1);
        pollFds_.pop_back();
        events_[lastFd]->setIndex(index);
    }
    event.setIndex(-event.fd() - 1);
}

void PollPoller::fillActiveEvents(int numEvents, EventList& activeEvents) const {
    for(auto& pfd : pollFds_) {
        if(pfd.revents > 0) {
            Event* event = events_.at(pfd.fd);
            if(event->fd() != pfd.fd) {
                LOG_ERROR("Event fd {} not equal to fd {}", event->fd(), pfd.fd);
            }
            event->setHappenedEvent(pfd.revents);
            activeEvents.push_back(event);
            --numEvents;
        }
        if(numEvents == 0) break;
    }
}