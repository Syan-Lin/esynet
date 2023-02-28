#pragma once

/* Standard headers */
#include <functional>
#include <atomic>

/* Local headers */
#include "net/Event.h"
#include "net/base/Socket.h"
#include "utils/NonCopyable.h"

namespace esynet {

class InetAddress;
class Reactor;

/* 非线程安全 */
class Acceptor : public utils::NonCopyable {
private:
    using ConnectionCallback = std::function<void(Socket, const InetAddress&)>;

public:
    Acceptor(Reactor&, const InetAddress& localAddr);
    ~Acceptor();

    void setConnectionCallback(ConnectionCallback);
    bool listening(); /* 线程安全 */
    void listen();

private:
    void OnConnection();

    Reactor& reactor_;
    Socket acceptSocket_;
    Event acceptEvent_;
    ConnectionCallback connCb_;
    std::atomic<bool> listen_;
    const int port_;
};

} /* namespace esynet */