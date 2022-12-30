#include <iostream>
#include <map>

#include "net/timer/Timer.h"
#include "net/timer/TimerQueue.h"
#include "net/EventLoop.h"
#include "net/Event.h"
#include "net/poller/EpollPoller.h"

#include <sys/timerfd.h>
#include <time.h>

using namespace std;

void print() {
    cout << "hello" << endl;
}

int main() {
    EventLoop loop;
    TimerQueue tq(loop);

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    Timestamp nowts(now.tv_sec * 1000000 + now.tv_nsec / 1000);

    tq.addTimer(print, nowts + 1000, 0);
    tq.addTimer(print, nowts, 1000);

    loop.loop();

    return 0;
}