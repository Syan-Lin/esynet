#include <ctime>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <cstdio>
#include <sys/timerfd.h>
#include "net/EventLoop.h"
#include "net/Event.h"

/* 使用 timefd 来测试 EventLoop 及其相关类 */

std::string gStrForTest;

void timeOut(EventLoop& loop) {
    gStrForTest += "t";
    loop.stop();
}

/* TODO: EventLoop_Test */
TEST_CASE("EventLoop_Test"){
    SUBCASE("PollPoller") {
        int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        struct itimerspec howlong;
        bzero(&howlong, sizeof howlong);
        howlong.it_value.tv_sec = 2;
        timerfd_settime(timer_fd, 0, &howlong, NULL);

        EventLoop loop(false);
        Event event(loop, timer_fd);
        event.setReadCallback(std::bind(timeOut, std::ref(loop)));
        event.enableReading(); /* 会委托 loop 更新 poll */

        loop.loop();
        close(timer_fd);
        CHECK(gStrForTest == "t");
    }

    SUBCASE("EpollPoller") {
        EventLoop loop;
    }
}