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

/* 非线程安全
 * 负责建立连接：
 * 当连接建立成功时，将监听权移交出去
 * 当连接建立失败时，进行重试 */
class Connector : public utils::NonCopyable {
private:
    using ConnectionCallback = std::function<void(Socket)>;
    enum State { kDisconnected, kConnecting, kConnected };

    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

public:
    Connector(Reactor&, const InetAddress& serverAddr);
    ~Connector();

    void setConnectionCallback(ConnectionCallback);
    void start();
    void restart();
    void stop();

    const InetAddress& serverAddr() const;

private:
    void OnConnection(Socket);
    void checkConnect(Socket);
    void retry(Socket);

    Reactor& reactor_;
    InetAddress serverAddr_;
    bool tryToConnect_;
    State state_;
    std::unique_ptr<Event> event_;
    ConnectionCallback connCb_;
    int retryDelayMs_;
};

} /* namespace esynet */