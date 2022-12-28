#include "net/poller/EpollPoller.h"

EpollPoller::EpollPoller(EventLoop& loop)
    : Poller(loop),
      epollFd_(epoll_create1(EPOLL_CLOEXEC)),
      epollEvents_(kInitEventListSize) {
    if(epollFd_ < 0) {
        LOG_FATAL("epoll_create1 error");
    }
}

EpollPoller::~EpollPoller() { close(epollFd_); }

Timestamp EpollPoller::poll(ChannelList& activeChannels, int timeoutMs) {
    int numEvents = epoll_wait(epollFd_, &*epollEvents_.begin(), epollEvents_.size(), timeoutMs);
    Timestamp pollTime(Timestamp::now());
    if(numEvents > 0) {
        LOG_DEBUG("{} events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    } else if(numEvents == 0) {
        LOG_DEBUG("Nothing happened");
    } else {
        LOG_ERROR("Poller poll error");
    }
    return pollTime;
}

void EpollPoller::updateChannel(Channel& channel) {
    LOG_DEBUG("Update channel(fd: {}, events: {})", channel.fd(), channel.events());
    if(channel.index() < 0) {
        /* 添加至列表 */
        if(channels_.find(channel.fd()) != channels_.end()) {
            LOG_ERROR("Channel(fd: {}) already exists", channel.fd());
        }
        epollUpdate(EPOLL_CTL_ADD, channel);
    } else {
        /* 更新列表 */
        if(channels_.find(channel.fd()) == channels_.end()) {
            LOG_ERROR("Channel(fd: {}) not exists", channel.fd());
        } else if(channels_[channel.fd()] != &channel) {
            LOG_ERROR("Channel(fd: {}) not the same", channel.fd());
        }
        epollUpdate(EPOLL_CTL_MOD, channel);
    }
}

void EpollPoller::removeChannel(Channel& channel) {
    LOG_DEBUG("Remove channel(fd: {})", channel.fd());
    if(channel.index() < 0) {
        return;
    } else {
        if(channels_.find(channel.fd()) == channels_.end()) {
            LOG_ERROR("Channel(fd: {}) not exists", channel.fd());
        } else if(channels_[channel.fd()] != &channel) {
            LOG_ERROR("Channel(fd: {}) not the same", channel.fd());
        }
        epollUpdate(EPOLL_CTL_DEL, channel);
    }
}

void EpollPoller::epollUpdate(int operation, Channel& channel) {
    EpollEvent event;
    event.events = channel.events();
    event.data.fd = channel.fd();
    if(epoll_ctl(epollFd_, operation, channel.fd(), &event) < 0) {
        LOG_ERROR("epoll_ctl({}) error", operation);
    }
    if(operation == EPOLL_CTL_ADD) {
        epollEvents_.push_back(event);
        channel.setIndex(epollEvents_.size() - 1);
        channels_[channel.fd()] = &channel;
    } else if(operation == EPOLL_CTL_DEL) {
        channels_.erase(channel.fd());
        int index = channel.index();
        if(index >= epollEvents_.size()) {
            LOG_ERROR("Channel(fd: {}) index out of range", channel.fd());
        }
        epollEvents_.erase(epollEvents_.begin() + index);
        channel.setIndex(-channel.fd() - 1);
    }
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList& activeChannels) const {
    for(int i = 0; i < numEvents; ++i) {
        Channel* channel = channels_.at(epollEvents_[i].data.fd);
        channel->setRevents(epollEvents_[i].events);
        activeChannels.push_back(channel);
    }
}