#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <cstdio>
#include "net/EventLoop.h"
#include "net/Event.h"

/* TODO: Event_Test */
TEST_CASE("Event_Test"){
    EventLoop loop;
}