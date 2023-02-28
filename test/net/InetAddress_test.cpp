#include "net/base/InetAddress.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include "net/base/Socket.h"
#include "logger/Logger.h"

using namespace esynet;

TEST_CASE("InetAddress_Test"){
    Logger::setLogger([](const std::string& msg) {});

    Socket sock;
    auto addr = InetAddress::getLocalAddr(sock);
    CHECK(addr.has_value());
    CHECK(addr->ip() == "0.0.0.0");
    CHECK(addr->port() == 0);

    Socket sock2;
    InetAddress addr2("0.0.0.0", 1234);
    sock2.bind(addr2);
    auto addr3 = InetAddress::getLocalAddr(sock2);
    CHECK(addr3.has_value());
    CHECK(addr3->ip() == "0.0.0.0");
    CHECK(addr3->port() == 1234);

    Socket sock3;
    InetAddress ad("0.0.0.0", 4567);
    InetAddress local("0.0.0.0", 12345);
    sock3.bind(local);
    sock3.connect(ad);
    auto addr4 = InetAddress::getLocalAddr(sock3);
    CHECK(addr4.has_value());
    CHECK(addr4->ip() == "127.0.0.1");
    CHECK(addr4->port() == 12345);
}