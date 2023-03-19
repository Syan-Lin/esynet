#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include <sys/timerfd.h>
#include "net/Looper.h"
#include "net/Event.h"

using namespace esynet;
using namespace esynet::utils;

int global = 0;

void test(Looper& loop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    loop.run([&loop] {
        global++;
        loop.stop();
    });
}

TEST_CASE("EventLoop_wakeup_Test"){
    Looper loop;
    std::thread t(test, std::ref(loop));

    Timestamp t1 = Timestamp::now();
    loop.start();
    Timestamp t2 = Timestamp::now();
    CHECK(t2 - t1 >= 3500);

    t.join();
}