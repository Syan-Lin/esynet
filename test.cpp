#include <iostream>
#include <map>

#include "net/timer/Timer.h"
#include "net/timer/TimerQueue.h"
#include "net/EventLoop.h"
#include "net/Event.h"
#include "net/poller/EpollPoller.h"

#include <sys/timerfd.h>
#include <dbg.h>
#include <time.h>

using namespace std;

using namespace esynet;
using namespace esynet::timer;
using namespace esynet::utils;

int g_test = 0;
int error = 1;  /* 误差1毫秒 */
Timestamp g_record;

void callonce() {
    int64_t duration = g_record.microSecondsSinceEpoch();
    g_record = Timestamp::now();
    duration = g_record.microSecondsSinceEpoch() - duration;
    g_test++;
}

void callback(EventLoop& loop, int id, TimerQueue& tq) {
    int64_t duration = g_record.microSecondsSinceEpoch();
    g_record = Timestamp::now();
    duration = g_record.microSecondsSinceEpoch() - duration;
    bool condition = (duration / 1000 >= 1000 - error) && (duration / 1000 <= 1000 + error);
    if(g_test == 1) {
        tq.cancel(id);
    } else if(g_test == 10) {
        loop.stop();
    }
    g_test++;
}

int main() {
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
    duration /= 1000;
    dbg(duration);

    return 0;
}