#include <chrono>
#include <iostream>
#include <map>

#include "net/timer/Timer.h"
#include "net/timer/TimerQueue.h"
#include "net/base/Looper.h"
#include "net/base/Event.h"
#include "net/thread/ReactorThreadPoll.h"
#include "net/poller/EpollPoller.h"
#include "net/base/Socket.h"

#include <sys/timerfd.h>
#include <dbg.h>
#include <thread>
#include <time.h>

using namespace std;

using namespace esynet;
using namespace esynet::timer;
using namespace esynet::utils;

int main() {
    Looper loop;
    ReactorThreadPoll poll(loop);

    Socket sock;

    poll.setThreadNum(4);
    poll.start();
    sleep(1);
    poll.stop();

    return 0;
}