#pragma once

/* Standard headers */
#include <functional>

/* Linux headers */
#include <sys/epoll.h>

/* Local headers */
#include "utils/NonCopyable.h"

class EventLoop;

class Channel : public NonCopyable {
public:
    using Callback = std::function<void()>;

public:
    Channel(EventLoop&, int);

    void handleEvent();
    void setReadCallback(Callback);
    void setWriteCallback(Callback);
    void setErrorCallback(Callback);

    short fd() const;
    short events() const;
    void setRevents(int);

    void enableReading();
    void enableWriting();
    void disableWriting();
    void disableReading();
    void disableAll();

    int index() const;
    void setIndex(int);

    EventLoop* ownerLoop() const;

private:
    /* EPOLLIN 等宏定义应该与 POLLIN 等宏定义一致 */
    enum Event {
        kNoneEvent = -1,
        kReadEvent = EPOLLIN | EPOLLPRI,
        kWriteEvent = EPOLLOUT,
        kErrorEvent = EPOLLERR
    };

private:
    void update();

    EventLoop& loop_;
    const int fd_;
    short events_;
    short revents_;
    int index_;

    Callback readCallback_;
    Callback writeCallback_;
    Callback errorCallback_;
};