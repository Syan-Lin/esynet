#pragma once

/* Standard headers */
#include <functional>
#include <atomic>

/* Local headers */
#include "net/base/Event.h"
#include "net/base/Socket.h"
#include "utils/NonCopyable.h"

namespace esynet {

class InetAddress;
class Reactor;

class Acceptor : public utils::NonCopyable {
private:
    using AcceptCallback = std::function<void(Socket, const InetAddress&)>;

public:
    Acceptor(Reactor&, const InetAddress& localAddr);
    ~Acceptor();

    void setAcceptCallback(AcceptCallback);
    bool listening() const;
    void listen();

private:
    void onAccept();

    Reactor& reactor_;
    Socket acceptSocket_;
    Event acceptEvent_;
    AcceptCallback acceptCb_;
    std::atomic<bool> listen_;
    const int port_;
};

} /* namespace esynet */