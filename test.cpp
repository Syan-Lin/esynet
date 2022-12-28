#include <iostream>

#include "net/EventLoop.h"
#include "net/Event.h"

using namespace std;

int main() {
    EventLoop loop(false);
    Event event(loop, 1);

    return 0;
}