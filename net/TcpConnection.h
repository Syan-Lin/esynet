#pragma once

/* Standard headers */
#include <any>

/* Local headers */
#include "net/Event.h"
#include "net/EventLoop.h"
#include "net/base/Socket.h"
#include "utils/Buffer.h"
#include "utils/Timestamp.h"

namespace esynet {

/* 线程安全考虑 */
class TcpConnection {
private:
    using TcpInfo = Socket::TcpInfo;

    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback = ConnectionCallback;
    using WriteCompleteCallback = ConnectionCallback;
    using MessageCallback = std::function<void(const TcpConnection&, utils::Buffer*, utils::Timestamp)>;
    using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

    enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };

public:
    TcpConnection(EventLoop& loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    const std::string&  name()          const;
    const InetAddress&  localAddress()  const;
    const InetAddress&  peerAddress()   const;
    bool                connected()     const;
    bool                disconnected()  const;
    TcpInfo             getTcpInfo()    const;
    std::string         getTcpInfoString() const;

    void send(const utils::StringPiece&);
    void shutdown(); // NOT thread safe
    void forceClose();
    void setTcpNoDelay(bool);
    void startRead();
    void stopRead();

    void setContext(const std::any& context);
    const std::any& getContext() const;

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setHighWaterMarkCallback(const HighWaterMarkCallback&, size_t highWaterMark);
    void setCloseCallback(const CloseCallback&);

    utils::Buffer& inputBuffer();
    utils::Buffer& outputBuffer();

    /* 当 TcpServer 建立新连接时调用 */
    void connectEstablished();
    /* 当 TcpServer 删除该连接时调用 */
    void connectDestroyed();

private:
    void handleRead(utils::Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void setState(State);
    std::string stateToString() const;

private:
    EventLoop& loop_;
    const std::string name_;

    Socket socket_;
    State state_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;
    utils::Buffer inputBuffer_;
    utils::Buffer outputBuffer_;
    std::any context_;
};

} /* namespace esynet */