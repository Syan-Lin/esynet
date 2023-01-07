#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <thread>
#include "net/base/Socket.h"
#include "net/base/InetAddress.h"

using namespace esynet;
using namespace esynet::utils;

#include <dbg.h>

TEST_CASE("InetAddress_Test"){
    InetAddress addr(8888);
    InetAddress addr2("127.0.0.1", 7777);

    CHECK(addr.ip() == "0.0.0.0");
    CHECK(addr2.ip() == "127.0.0.1");
    CHECK(addr.port() == 8888);
    CHECK(addr2.port() == 7777);

    InetAddress::SockAddrIn sockaddr = *reinterpret_cast
            <const InetAddress::SockAddrIn*>(&addr2.getSockAddr());
    sockaddr.sin_port = htons(7891);
    addr2.setSockAddr(*reinterpret_cast<const InetAddress::SockAddr*>(&sockaddr));
    CHECK(addr2.port() == 7891);

    auto hostaddr = InetAddress::resolve("github.com");
}

TEST_CASE("Socket_Test"){
    std::thread client([] {
        sleep(1);
        Socket client;
        InetAddress addr("127.0.0.1", 8888);
        client.connect(addr);
    });

    Socket sock;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    Socket sock2(sockfd);

    CHECK(sock2.fd() == sockfd);
    InetAddress local(8888);
    sock.bind(local);
    sock.listen();
    InetAddress peerAddr(-1);
    sock.accept(peerAddr);
    CHECK(peerAddr.ip() == "0.0.0.0");
    CHECK(peerAddr.port() == 8888);

    client.join();
}