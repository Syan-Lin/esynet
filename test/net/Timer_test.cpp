#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include "net/timer/Timer.h"

using namespace esynet::timer;

int g_test = 0;

void callback() {
    g_test++;
}

TEST_CASE("Timer_Test"){
    Timer timer0(callback, Timestamp() + 1, 0);
    Timer timer1(callback, Timestamp() + 2, 10.5);

    CHECK(timer0.expiration() == Timestamp() + 1);
    CHECK(timer1.expiration() == Timestamp() + 2);
    CHECK(timer0.repeat() == false);
    CHECK(timer1.repeat() == true);
    CHECK(timer0.id() == 0);
    CHECK(timer1.id() == 1);
    timer0.run();
    timer1.run();

    timer0.restart();
    timer1.restart();
    CHECK(timer0.expiration() == Timestamp());
    CHECK(timer1.expiration() > Timestamp::now());
    CHECK(g_test == 2);
}