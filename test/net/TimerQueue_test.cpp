#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include "net/timer/TimerQueue.h"
#include "net/EventLoop.h"

#include <dbg.h>

using namespace esynet;
using namespace esynet::timer;

int g_test = 0;
int error = 1;  /* 误差1毫秒 */
Timestamp g_record;

void callonce() {
    int64_t duration = g_record.microSecondsSinceEpoch();
    g_record = Timestamp::now();
    duration = g_record.microSecondsSinceEpoch() - duration;
    CHECK(duration / 1000 == 0);
    g_test++;
}

void callback(EventLoop& loop, int id, TimerQueue& tq) {
    int64_t duration = g_record.microSecondsSinceEpoch();
    g_record = Timestamp::now();
    duration = g_record.microSecondsSinceEpoch() - duration;
    bool condition = (duration / 1000 >= 1000 - error) && (duration / 1000 <= 1000 + error);
    CHECK(condition);
    if(g_test == 1) {
        tq.cancel(id);
    } else if(g_test == 3) {
        loop.stop();
    }
    g_test++;
}

TEST_CASE("TimerQueue_Test"){
    EventLoop loop;
    TimerQueue tq(loop);

    Timestamp t1 = Timestamp::now();

    /* 立刻调用一次 */
    int id = tq.addTimer(callonce, Timestamp::now(), 2000);

    /* 一秒后每秒执行一次 */
    tq.addTimer(std::bind(callback, std::ref(loop), id, std::ref(tq)), Timestamp::now() + 1000, 1000);

    g_record = Timestamp::now();
    loop.loop();

    Timestamp t2 = Timestamp::now();
    int duration = t2.microSecondsSinceEpoch() - t1.microSecondsSinceEpoch();
    bool condition = (duration / 1000 >= 3000 - error) && (duration / 1000 <= 3000 + error);
    CHECK(condition);
}