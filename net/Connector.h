#pragma once

/* Standard headers */
#include <functional>
#include <atomic>

/* Local headers */
#include "net/base/Socket.h"
#include "utils/NonCopyable.h"
#include "net/base/InetAddress.h"

namespace esynet {

class InetAddress;
class Reactor;
class Event;

class Connector : public utils::NonCopyable {
private:
    using ConnectCallback = std::function<void(Socket)>;
    enum State { kDisconnected, kConnecting, kConnected };

    static const int kMaxRetryDelayMs;
    static const int kInitRetryDelayMs;

public:
    Connector(Reactor&, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddr() const;
    void setConnectCallback(ConnectCallback);

private:
    void onConnect(Socket);
    void checkConnect(Socket);
    void retry(Socket);

    Reactor& reactor_;
    InetAddress serverAddr_;
    bool tryToConnect_;
    State state_;
    std::unique_ptr<Event> event_;
    ConnectCallback connectCb_;
    int retryDelayMs_;
};

} /* namespace esynet */