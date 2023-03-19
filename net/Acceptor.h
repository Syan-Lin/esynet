#pragma once

/* Standard headers */
#include <functional>
#include <atomic>

/* Local headers */
#include "net/base/Event.h"
#include "net/base/Socket.h"
#include "utils/NonCopyable.h"

namespace esynet {

class NetAddress;
class Looper;

class Acceptor : public utils::NonCopyable {
private:
    using AcceptCallback = std::function<void(Socket, const NetAddress&)>;

public:
    Acceptor(Looper&, const NetAddress& localAddr);
    ~Acceptor();

    void setAcceptCallback(AcceptCallback);
    bool listening() const;
    void listen();

private:
    void onAccept();

    Looper& looper_;
    Socket acceptSocket_;
    Event acceptEvent_;
    AcceptCallback acceptCb_;
    std::atomic<bool> listen_;
    const int port_;
};

} /* namespace esynet */