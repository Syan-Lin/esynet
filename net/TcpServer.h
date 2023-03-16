#pragma once

/* Local headers */
#include "net/Acceptor.h"
#include "net/thread/ReactorThreadPoll.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "net/TcpConnection.h"
#include "net/base/Socket.h"
#include "net/base/InetAddress.h"

namespace esynet {

class Reactor;

class TcpServer : public utils::NonCopyable {
private:
    using TcpConnectionPtr      = TcpConnection::TcpConnectionPtr;
    using ConnectionMap         = std::map<std::string, TcpConnectionPtr>;

    using ConnectionCallback    = TcpConnection::ConnectionCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;
    using MessageCallback       = TcpConnection::MessageCallback;
    using CloseCallback         = TcpConnection::CloseCallback;
    using ErrorCallback         = TcpConnection::ErrorCallback;
    using ThreadInitCallback    = std::function<void(Reactor&)>;

public:
    enum Strategy { kRoundRobin, kLightest };

public:
    TcpServer(Reactor&, InetAddress addr = 8080, utils::StringPiece name = "Server");
    ~TcpServer();

    const std::string&  ip() const;
    const std::string&  name() const;
    int                 port() const;
    Reactor&            reactor();

    /* 非线程安全 */
    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setCloseCallback(const CloseCallback&);
    void setErrorCallback(const ErrorCallback&);
    void setThreadInitCallback(const ThreadInitCallback&);
    void setThreadNumInPool(size_t numThreads = 0);

    ReactorThreadPoll& threadPoll();
    void setThreadPollStrategy(Strategy strategy);

    void start(); /* 线程安全，开始监听 */

private:
    void onConnection(Socket, const InetAddress&);
    void removeConnection(TcpConnection& conn);

    Reactor& reactor_;
    const int port_;
    const std::string ip_;
    const std::string name_;
    std::atomic<bool> started_;

    Acceptor acceptor_;
    ReactorThreadPoll threadPoll_;
    Strategy strategy_ = kRoundRobin;

    ConnectionCallback connectionCb_;
    WriteCompleteCallback writeCompleteCb_;
    MessageCallback messageCb_;
    CloseCallback closeCb_;
    ErrorCallback errorCb_;

    int nextConnId_;
    ConnectionMap connections_;
};

} /* namespace esynet */