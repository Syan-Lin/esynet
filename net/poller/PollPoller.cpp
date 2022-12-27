#include "net/poller/PollPoller.h"
#include "net/Channel.h"
#include "net/EventLoop.h"

PollPoller::PollPoller(EventLoop& loop) : Poller(loop) {}

Timestamp PollPoller::poll(ChannelList& activeChannels, int timeoutMs) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
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

void PollPoller::updateChannel(Channel& channel) {
    LOG_DEBUG("Update channel(fd: {}, events: {})", channel.fd(), channel.events());
    if(channel.index() < 0) {
        /* 添加至列表 */
        if(channels_.find(channel.fd()) != channels_.end()) {
            LOG_ERROR("Channel(fd: {}) already exists", channel.fd());
        }
        PollFd pfd;
        pfd.fd = channel.fd();
        pfd.events = channel.events();
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        channel.setIndex(pollfds_.size() - 1);
        channels_[pfd.fd] = &channel;
    } else {
        /* 更新列表 */
        if(channels_.find(channel.fd()) == channels_.end()) {
            LOG_ERROR("Channel(fd: {}) not exists", channel.fd());
        } else if(channels_[channel.fd()] != &channel) {
            LOG_ERROR("Channel(fd: {}) not the same", channel.fd());
        }
        PollFd& pfd = pollfds_.at(channel.index());
        pfd.events = channel.events();
        pfd.revents = 0;
    }
}

void PollPoller::removeChannel(Channel& channel) {
    LOG_DEBUG("Remove channel(fd: {})", channel.fd());
    if(channel.index() < 0) {
        return;
    } else {
        if(channels_.find(channel.fd()) == channels_.end()) {
            LOG_ERROR("Channel(fd: {}) not exists", channel.fd());
        } else if(channels_[channel.fd()] != &channel) {
            LOG_ERROR("Channel(fd: {}) not the same", channel.fd());
        }
        channels_.erase(channel.fd());
        int index = channel.index();
        if(index >= pollfds_.size()) {
            LOG_ERROR("Channel(fd: {}) index out of range", channel.fd());
        }
        pollfds_.erase(pollfds_.begin() + index);
        channel.setIndex(-1);
    }
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList& activeChannels) const {
    for(auto& pfd : pollfds_) {
        if(pfd.revents > 0) {
            Channel* channel = channels_.at(pfd.fd);
            if(channel->fd() != pfd.fd) {
                LOG_ERROR("Channel fd {} not equal to fd {}", channel->fd(), pfd.fd);
            }
            channel->setRevents(pfd.revents);
            activeChannels.push_back(channel);
            --numEvents;
        }
        if(numEvents == 0) break;
    }
}