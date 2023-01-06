#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include <sys/timerfd.h>
#include "net/EventLoop.h"
#include "net/Event.h"

using namespace esynet;
using namespace esynet::utils;

int global = 0;

void test(EventLoop& loop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    loop.runInLoop([&loop] {
        global++;
        loop.stop();
    });
}

TEST_CASE("EventLoop_wakeup_Test"){
    EventLoop loop;
    std::thread t(test, std::ref(loop));

    Timestamp t1 = Timestamp::now();
    loop.loop();
    Timestamp t2 = Timestamp::now();
    CHECK(t2 - t1 >= 3500);

    t.join();
}