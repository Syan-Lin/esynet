#pragma once

/* Local headers */
#include "net/base/Acceptor.h"
#include "net/thread/EventLoopThreadPoll.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "net/EventLoop.h"
#include "net/TcpConnection.h"
#include "net/base/Socket.h"

namespace esynet {

class TcpServer : public utils::NonCopyable {
private:
    using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    using ConnectionCallback = TcpConnection::ConnectionCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;
    using MessageCallback = TcpConnection::MessageCallback;
    using ThreadInitCallback = std::function<void(EventLoop&)>;

public:
    enum Strategy { kRoundRobin, kLightest };

public:
    TcpServer(EventLoop&, InetAddress addr = 8080, utils::StringPiece name = "Server");
    ~TcpServer();

    const std::string&  ip() const;
    const std::string&  name() const;
    int                 port() const;
    EventLoop&          getLoop();

    /* 非线程安全 */
    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setThreadInitCallback(const ThreadInitCallback&);
    void setThreadNumInPool(size_t numThreads = 0);

    EventLoopThreadPoll& threadPoll();
    void setThreadPollStrategy(Strategy strategy);

    void start(); /* 线程安全，开始监听 */

private:
    /* 非线程安全 */
    void OnConnection(Socket, const InetAddress&);
    /* 线程安全 */
    void removeConnection(const TcpConnection& conn);

    EventLoop& loop_;
    const int port_;
    const std::string ip_;
    const std::string name_;
    std::atomic<bool> started_;

    Acceptor acceptor_;
    EventLoopThreadPoll threadPoll_;
    Strategy strategy_ = kRoundRobin;

    ConnectionCallback connectionCb_;
    WriteCompleteCallback writeCompleteCb_;
    MessageCallback messageCb_;

    int nextConnId_;
    ConnectionMap connections_;
};

} /* namespace esynet */