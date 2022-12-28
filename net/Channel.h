#pragma once

/* Standard headers */
#include <functional>

/* Linux headers */
#include <sys/epoll.h>

/* Local headers */
#include "utils/NonCopyable.h"

class EventLoop;

/* TODO: 名称改为Event */
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

    /* 设置监听事件 */
    void enableReading();
    void enableWriting();
    void disableWriting();
    void disableReading();
    void disableAll();

    /* Poller相关 */
    int index() const;
    void setIndex(int);

    EventLoop* ownerLoop() const;

private:
    /* EPOLLIN 等宏定义应该与 POLLIN 等宏定义一致 */
    enum {
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