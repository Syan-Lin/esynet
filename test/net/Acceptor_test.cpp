#include "net/base/NetAddress.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include <doctest/doctest.h>
#include <functional>
#include <sys/timerfd.h>
#include "net/base/Looper.h"
#include "net/base/Acceptor.h"
#include "logger/Logger.h"

using namespace esynet;

TEST_CASE("Acceptor_Test"){
    Looper looper;
    NetAddress listenAddr(8888);

    Acceptor acceptor(looper, listenAddr);
    acceptor.setAcceptCallback([](Socket connSock, const NetAddress& peerAddr){
        LOG_INFO("New connection from {}", peerAddr.ip());
        /* 可以省去，Socket保证会自动close，但可能延迟
         * 如果有立刻close的需求可以这样调用 */
        connSock.close();
    });
    acceptor.listen();

    looper.start();
}