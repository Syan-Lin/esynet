#include <chrono>
#include <iostream>
#include <map>

#include "net/timer/Timer.h"
#include "net/timer/TimerQueue.h"
#include "net/EventLoop.h"
#include "net/Event.h"
#include "net/thread/EventLoopThreadPoll.h"
#include "net/poller/EpollPoller.h"

#include <sys/timerfd.h>
#include <dbg.h>
#include <thread>
#include <time.h>

using namespace std;

using namespace esynet;
using namespace esynet::timer;
using namespace esynet::utils;

int main() {
    EventLoop loop;
    EventLoopThreadPoll poll(loop);

    poll.start(4);
    sleep(1);
    poll.stop();

    return 0;
}