#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <thread>
#include <sys/stat.h>
#include "net/base/Socket.h"
#include "net/base/NetAddress.h"

using namespace esynet;
using namespace esynet::utils;

TEST_CASE("InetAddress_Test"){
    NetAddress addr(8888);
    NetAddress addr2("127.0.0.1", 7777);

    CHECK(addr.ip() == "0.0.0.0");
    CHECK(addr2.ip() == "127.0.0.1");
    CHECK(addr.port() == 8888);
    CHECK(addr2.port() == 7777);

    NetAddress::SockAddrIn sockaddr = *reinterpret_cast
            <const NetAddress::SockAddrIn*>(&addr2.getSockAddr());
    sockaddr.sin_port = htons(7891);
    addr2.setSockAddr(*reinterpret_cast<const NetAddress::SockAddr*>(&sockaddr));
    CHECK(addr2.port() == 7891);

    auto hostaddr = NetAddress::resolve("github.com");
}

TEST_CASE("Socket_Test"){
    std::thread client([] {
        sleep(1);
        Socket client;
        NetAddress addr("127.0.0.1", 8888);
        auto ret = client.connect(addr);
    });

    Socket sock;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    Socket sock2(sockfd);

    CHECK(sock2.fd() == sockfd);
    NetAddress local(8888);
    sock.bind(local);
    sock.listen();
    NetAddress peerAddr(-1);
    auto ret = sock.accept(peerAddr);
    CHECK(peerAddr.ip() == "0.0.0.0");
    CHECK(peerAddr.port() == 8888);

    client.join();
    int temp;
    struct stat s;
    {
        Socket s1;
        temp = s1.fd();
        CHECK(fstat(s1.fd(), &s) != -1);

        Socket s2 = s1;
        Socket s3 = std::move(s2);
        Socket s4(s1);
        Socket s5(std::move(s1));
        CHECK(fstat(s5.fd(), &s) != -1);
    }
    CHECK(fstat(temp, &s) == -1);
}