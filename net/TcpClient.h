#pragma once

/* Local headers */
#include "net/base/Acceptor.h"
#include "net/thread/ReactorThreadPoll.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "net/TcpConnection.h"
#include "net/base/Socket.h"
#include "net/base/InetAddress.h"

namespace esynet {

class Reactor;
class Connector;

class TcpClient : public utils::NonCopyable {
private:
    using TcpConnectionPtr      = TcpConnection::TcpConnectionPtr;
    using ConnectorPtr          = std::unique_ptr<Connector>;
    using ConnectionCallback    = TcpConnection::ConnectionCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;
    using MessageCallback       = TcpConnection::MessageCallback;

public:
    TcpClient(Reactor&, InetAddress addr = 8080, utils::StringPiece name = "Server");
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();
    void enableRetry();

    TcpConnectionPtr    connection() const;
    Reactor&            reactor() const;
    const std::string&  name() const;
    void                willRetry() const;

    /* 非线程安全 */
    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);

    void start(); /* 线程安全，开始监听 */

private:
    void onConnection(Socket);
    void removeConnection(TcpConnection&);

    Reactor& reactor_;
    ConnectorPtr connector_;
    TcpConnectionPtr connection_;

    ConnectionCallback connectionCb_;
    MessageCallback messageCb_;
    WriteCompleteCallback writeCompleteCb_;

    const std::string name_;
    bool retry_;
    bool tryToConnect_;
    int nextConnId_;

    std::mutex mutex_;
};

} /* namespace esynet */