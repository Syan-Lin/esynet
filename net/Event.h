#pragma once

/* Standard headers */
#include <functional>

/* Linux headers */
#include <sys/poll.h>

/* Local headers */
#include "utils/NonCopyable.h"

namespace esynet {

class EventLoop;

class Event : public utils::NonCopyable {
public:
    using Callback = std::function<void()>;

public:
    Event(EventLoop&, int);

    void handle();
    void setCloseCallback(Callback);
    void setReadCallback(Callback);
    void setWriteCallback(Callback);
    void setErrorCallback(Callback);

    int fd() const;
    short listenedEvent() const;
    bool writable() const;
    bool readable() const;
    void setHappenedEvent(int event);

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
        kReadEvent = POLLIN | POLLPRI,
        kErrorEvent = POLLERR | POLLNVAL,
        kWriteEvent = POLLOUT,
        kCloseEvent = POLLHUP | POLLRDHUP,
    };

private:
    void update();

    EventLoop& loop_;
    const int fd_;
    short listenedEvents_;
    short happenedEvents_;
    int indexInPoll_;

    Callback readCallback_;
    Callback writeCallback_;
    Callback errorCallback_;
    Callback closeCallback_;
};

} /* namespace esynet */