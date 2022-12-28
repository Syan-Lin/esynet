#include "net/poller/PollPoller.h"
#include "logger/Logger.h"

PollPoller::PollPoller(EventLoop& loop) : Poller(loop) {}

Timestamp PollPoller::poll(EventList& activeEvents, int timeoutMs) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
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

void PollPoller::updateEvent(Event& event) {
    LOG_DEBUG("Update event(fd: {}, events: {})", event.fd(), event.listenedEvent());
    if(event.index() < 0) {
        /* 添加至列表 */
        if(events_.find(event.fd()) != events_.end()) {
            LOG_ERROR("Event(fd: {}) already exists", event.fd());
        }
        PollFd pfd;
        pfd.fd = event.fd();
        pfd.events = event.listenedEvent();
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        event.setIndex(pollfds_.size() - 1);
        events_[pfd.fd] = &event;
    } else {
        /* 更新列表 */
        if(events_.find(event.fd()) == events_.end()) {
            LOG_ERROR("Event(fd: {}) not exists", event.fd());
        } else if(events_[event.fd()] != &event) {
            LOG_ERROR("Event(fd: {}) not the same", event.fd());
        }
        PollFd& pfd = pollfds_.at(event.index());
        pfd.events = event.listenedEvent();
        pfd.revents = 0;
    }
}

void PollPoller::removeEvent(Event& event) {
    LOG_DEBUG("Remove event(fd: {})", event.fd());
    if(event.index() < 0) {
        return;
    } else {
        if(events_.find(event.fd()) == events_.end()) {
            LOG_ERROR("Event(fd: {}) not exists", event.fd());
        } else if(events_[event.fd()] != &event) {
            LOG_ERROR("Event(fd: {}) not the same", event.fd());
        }
        events_.erase(event.fd());
        int index = event.index();
        if(index >= pollfds_.size()) {
            LOG_ERROR("Event(fd: {}) index out of range", event.fd());
        }
        pollfds_.erase(pollfds_.begin() + index);
        event.setIndex(-event.fd() - 1);
    }
}

void PollPoller::fillActiveEvents(int numEvents, EventList& activeEvents) const {
    for(auto& pfd : pollfds_) {
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