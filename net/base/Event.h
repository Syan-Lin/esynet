#pragma once

/* Standard headers */
#include <functional>

/* Linux headers */
#include <sys/poll.h>

/* Local headers */
#include "utils/NonCopyable.h"

namespace esynet {

class Looper;

class Event : public utils::NonCopyable {
public:
    using Callback = std::function<void()>;

public:
    Event(Looper&, int fd);

    void handle();
    void setCloseCallback(Callback);
    void setReadCallback(Callback);
    void setWriteCallback(Callback);
    void setErrorCallback(Callback);

    int   fd()            const;
    short listenedEvent() const;
    bool  writable()      const;
    bool  readable()      const;
    void  setHappenedEvent(int event);

    /* 设置监听事件 */
    void enableRead();
    void enableWrite();
    void disableWrite();
    void disableRead();
    void cancel();

    /* Poller */
    int index() const;
    void setIndex(int);

    Looper* looper() const;

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

    const int fd_;
    Looper& looper_;
    int   indexInPoll_    {-1};
    short listenedEvents_ {-1};
    short happenedEvents_ {0};

    Callback readCallback_;
    Callback writeCallback_;
    Callback errorCallback_;
    Callback closeCallback_;
};

} /* namespace esynet */