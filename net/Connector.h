#pragma once

/* Standard headers */
#include <functional>
#include <atomic>

/* Local headers */
#include "net/base/Socket.h"
#include "utils/NonCopyable.h"
#include "net/base/NetAddress.h"

namespace esynet {

class NetAddress;
class Looper;
class Event;

class Connector : public utils::NonCopyable {
private:
    using ConnectCallback = std::function<void(Socket, NetAddress)>;
    enum State { kDisconnected, kConnecting, kConnected };

    static const int kMaxRetryDelayMs;
    static const int kInitRetryDelayMs;

public:
    Connector(Looper&, const NetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    void setConnectCallback(ConnectCallback);

private:
    void onConnect(Socket, NetAddress);
    void checkConnect(Socket);
    void retry(Socket);

    Looper& looper_;
    NetAddress serverAddr_;
    ConnectCallback connectCb_;
    std::unique_ptr<Event> event_;
    bool  couldConnect  {true};
    State state_        {kDisconnected};
    int   retryDelayMs_ {kInitRetryDelayMs};
};

} /* namespace esynet */