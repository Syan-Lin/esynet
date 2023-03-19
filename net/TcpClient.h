#pragma once

/* Local headers */
#include "net/Acceptor.h"
#include "net/thread/ReactorThreadPoll.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "net/TcpConnection.h"
#include "net/base/Socket.h"
#include "net/base/NetAddress.h"

namespace esynet {

class Looper;
class Connector;

/* 非线程安全 */
class TcpClient : public utils::NonCopyable {
private:
    using TcpConnectionPtr      = TcpConnection::TcpConnectionPtr;
    using ConnectorPtr          = std::unique_ptr<Connector>;
    using ConnectionCallback    = TcpConnection::ConnectionCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;
    using MessageCallback       = TcpConnection::MessageCallback;
    using CloseCallback         = TcpConnection::CloseCallback;
    using ErrorCallback         = TcpConnection::ErrorCallback;

public:
    TcpClient(NetAddress, utils::StringPiece name = "Client", bool useEpoll = true);
    ~TcpClient();

    void connect();
    void disconnect();
    void reconnect();
    void stopConnecting();

    /* 线程安全 */
    TcpConnectionPtr    connection() const;
    Looper&            looper();
    const std::string&  name() const;

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setCloseCallback(const CloseCallback&);
    void setErrorCallback(const ErrorCallback&);

    void start();
    void shutdown();

private:
    void onConnection(Socket, NetAddress);
    void removeConnection(TcpConnection&);

    Looper looper_;
    ConnectorPtr connector_;
    TcpConnectionPtr connection_;

    ConnectionCallback connectionCb_;
    MessageCallback messageCb_;
    CloseCallback closeCb_;
    ErrorCallback errorCb_;
    WriteCompleteCallback writeCompleteCb_;

    const std::string name_;
};

} /* namespace esynet */