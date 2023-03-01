#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include <sys/timerfd.h>
#include "net/Reactor.h"
#include "net/Event.h"

using namespace esynet;

/* 使用 time_fd 来测试 Reactor 及其相关类 */

std::string gStrForTest;

void readCallBack(Event& event) {
    gStrForTest += "r";
    event.disableRead();
    event.disableRead();
}
void writeCallBack(Event& event) {
    gStrForTest += "w";
    event.disableWrite();
    event.disableWrite();
}
void errorCallBack(Event& event) {
    gStrForTest += "e";
    event.disableAll();
    event.disableAll();
}
void stopLoop(Reactor& loop) {
    loop.stop();
}

TEST_CASE("EventLoop_Test"){
    SUBCASE("PollPoller") {
        gStrForTest.clear();
        Reactor loop(false);

        /* 用于关闭loop */
        int close_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        struct itimerspec closetime;
        bzero(&closetime, sizeof closetime);
        closetime.it_value.tv_sec = 2;
        timerfd_settime(close_fd, 0, &closetime, NULL);
        Event stopEvent(loop, close_fd);
        stopEvent.setReadCallback(std::bind(stopLoop, std::ref(loop)));
        stopEvent.enableRead();
        stopEvent.disableRead();
        stopEvent.enableRead();
        stopEvent.disableWrite();

        /* 测试读事件监听 */
        int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        struct itimerspec howlong;
        bzero(&howlong, sizeof howlong);
        howlong.it_value.tv_sec = 1;
        timerfd_settime(timer_fd, 0, &howlong, NULL);

        Event readEvent(loop, timer_fd);
        readEvent.setReadCallback(std::bind(readCallBack, std::ref(readEvent)));
        readEvent.enableRead(); /* 会委托 loop 更新 poll */

        /* 测试写事件监听 */
        int write_fd = fileno(stdout);

        Event writeEvent(loop, write_fd);
        writeEvent.setWriteCallback(std::bind(writeCallBack, std::ref(writeEvent)));
        writeEvent.enableWrite(); /* 会委托 loop 更新 poll */

        sleep(1);

        /* 测试错误事件监听 */
        Event errorEvent(loop, 1024);
        errorEvent.setErrorCallback(std::bind(errorCallBack, std::ref(errorEvent)));
        errorEvent.enableRead();

        loop.start();
        close(timer_fd);
        CHECK(gStrForTest == "rwe");
    }

    SUBCASE("EpollPoller") {
        gStrForTest.clear();
        Reactor loop;

        /* 用于关闭loop */
        int close_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        struct itimerspec closetime;
        bzero(&closetime, sizeof closetime);
        closetime.it_value.tv_sec = 2;
        timerfd_settime(close_fd, 0, &closetime, NULL);
        Event stopEvent(loop, close_fd);
        stopEvent.setReadCallback(std::bind(stopLoop, std::ref(loop)));
        stopEvent.enableRead();
        stopEvent.disableRead();
        stopEvent.enableRead();
        stopEvent.disableWrite();

        /* 测试读事件监听 */
        int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        struct itimerspec howlong;
        bzero(&howlong, sizeof howlong);
        howlong.it_value.tv_sec = 1;
        timerfd_settime(timer_fd, 0, &howlong, NULL);

        Event readEvent(loop, timer_fd);
        readEvent.setReadCallback(std::bind(readCallBack, std::ref(readEvent)));
        readEvent.enableRead(); /* 会委托 loop 更新 poll */

        /* 测试写事件监听 */
        int write_fd = fileno(stdout);

        Event writeEvent(loop, write_fd);
        writeEvent.setWriteCallback(std::bind(writeCallBack, std::ref(writeEvent)));
        writeEvent.enableWrite(); /* 会委托 loop 更新 poll */

        /* 测试错误事件监听 */
        Event errorEvent(loop, 1024);
        errorEvent.setErrorCallback(std::bind(errorCallBack, std::ref(errorEvent)));
        errorEvent.enableRead();

        loop.start();
        close(timer_fd);
        CHECK(gStrForTest == "ewr");
    }
}