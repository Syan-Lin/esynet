#pragma once

/* Local headers */
#include "net/EventLoop.h"
#include "net/Event.h"
#include "net/base/Socket.h"
#include "net/base/InetAddress.h"
#include "utils/NonCopyable.h"

namespace esynet {

class Acceptor : public utils::NonCopyable {
private:
    using ConnectionCallback = std::function<void(Socket, const InetAddress&)>;

public:
    Acceptor(EventLoop&, const InetAddress& localAddr);
    ~Acceptor();

    void setConnectionCallback(ConnectionCallback);
    bool listening();
    void listen();

private:
    void OnConnection();

    EventLoop& loop_;
    Socket acceptSocket_;
    Event acceptEvent_;
    ConnectionCallback connCb_;
    bool listen_;
};

} /* namespace esynet */