#include "net/base/NetAddress.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include "net/base/Socket.h"
#include "logger/Logger.h"

using namespace esynet;

TEST_CASE("InetAddress_Test"){
    Logger::setLogger([](const std::string& msg) {});

    Socket sock;
    auto addr = NetAddress::getLocalAddr(sock);
    CHECK(addr.has_value());
    CHECK(addr->ip() == "0.0.0.0");
    CHECK(addr->port() == 0);

    Socket sock2;
    NetAddress addr2("0.0.0.0", 1234);
    sock2.bind(addr2);
    auto addr3 = NetAddress::getLocalAddr(sock2);
    CHECK(addr3.has_value());
    CHECK(addr3->ip() == "0.0.0.0");
    CHECK(addr3->port() == 1234);

    Socket sock3;
    NetAddress ad("0.0.0.0", 4567);
    NetAddress local("0.0.0.0", 12345);
    sock3.bind(local);
    auto ret = sock3.connect(ad);
    auto addr4 = NetAddress::getLocalAddr(sock3);
    CHECK(addr4.has_value());
    CHECK(addr4->ip() == "127.0.0.1");
    CHECK(addr4->port() == 12345);
}