#include "net/base/InetAddress.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include <sys/timerfd.h>
#include "net/Reactor.h"
#include "net/base/Acceptor.h"
#include "logger/Logger.h"

using namespace esynet;

TEST_CASE("Acceptor_Test"){
    Reactor reactor;
    InetAddress listenAddr(8888);

    Acceptor acceptor(reactor, listenAddr);
    acceptor.setConnectionCallback([](Socket connSock, const InetAddress& peerAddr){
        LOG_INFO("New connection from {}", peerAddr.ip());
        /* 可以省去，Socket保证会自动close，但可能延迟
         * 如果有立刻close的需求可以这样调用 */
        connSock.close();
    });
    acceptor.listen();

    reactor.start();
}