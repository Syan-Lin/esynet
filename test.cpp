#include <iostream>

#include "net/EventLoop.h"
#include "net/Event.h"
#include "net/poller/EpollPoller.h"

using namespace std;

int main() {
    EventLoop loop(false);
    // Event event(loop, 1);
    // event.fd();

    return 0;
}