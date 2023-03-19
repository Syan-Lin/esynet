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

/* 非线程安全 */
class TcpServer : public utils::NonCopyable {
private:
    using TcpConnectionPtr      = TcpConnection::TcpConnectionPtr;
    using ConnectionMap         = std::map<std::string, TcpConnectionPtr>;

    using ConnectionCallback    = TcpConnection::ConnectionCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;
    using MessageCallback       = TcpConnection::MessageCallback;
    using CloseCallback         = TcpConnection::CloseCallback;
    using ErrorCallback         = TcpConnection::ErrorCallback;
    using ThreadInitCallback    = std::function<void(Looper&)>;

public:
    enum Strategy { kRoundRobin, kLightest };

public:
    TcpServer(NetAddress addr = 8080,
              utils::StringPiece name = "Server",
              bool useEpoll = true);
    ~TcpServer();

    /* 线程安全 */
    const std::string& ip()   const;
    const std::string& name() const;
    const int          port() const;
    Looper&           looper();

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setCloseCallback(const CloseCallback&);
    void setErrorCallback(const ErrorCallback&);
    void setThreadInitCallback(const ThreadInitCallback&);
    void setThreadNumInPool(size_t numThreads = 0);

    ReactorThreadPoll& threadPoll();
    void setThreadPollStrategy(Strategy strategy);

    void start();
    void shutdown();

private:
    void onConnection(Socket, const NetAddress&);
    void removeConnection(TcpConnection& conn);

    Looper looper_;
    const int port_;
    const std::string ip_;
    const std::string name_;
    std::atomic<bool> started_;

    Acceptor acceptor_;
    ReactorThreadPoll threadPoll_;
    Strategy strategy_;

    ConnectionCallback connectionCb_;
    WriteCompleteCallback writeCompleteCb_;
    MessageCallback messageCb_;
    CloseCallback closeCb_;
    ErrorCallback errorCb_;

    int connectionCount_;
    ConnectionMap connections_;
};

} /* namespace esynet */