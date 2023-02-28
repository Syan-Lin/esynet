#pragma once

/* Standard headers */
#include <any>
#include <memory>

/* Local headers */
#include "net/Event.h"
#include "net/EventLoop.h"
#include "net/base/Socket.h"
#include "utils/Buffer.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "utils/Timestamp.h"

namespace esynet {

/* 线程安全 */
class TcpConnection : utils::NonCopyable {
private:
    using TcpInfo = Socket::TcpInfo;
    enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };

public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnection&)>;
    using CloseCallback = ConnectionCallback;
    using WriteCompleteCallback = ConnectionCallback;
    using MessageCallback = std::function<void(const TcpConnection&, utils::Buffer&, utils::Timestamp)>;
    using HighWaterMarkCallback = std::function<void(const TcpConnection&, size_t)>;

public:
    static void defaultConnectionCallback(const TcpConnection&);
    static void defaultMessageCallback(const TcpConnection&, utils::Buffer&, utils::Timestamp);

public:
    TcpConnection(EventLoop& loop,
                utils::StringPiece name,
                Socket sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    /* 状态相关 */
    const std::string&  name()          const;
    const InetAddress&  localAddress()  const;
    const InetAddress&  peerAddress()   const;
    bool                connected()     const;
    bool                disconnected()  const;
    TcpInfo             getTcpInfo()    const;
    std::string         getTcpInfoString() const;
    EventLoop&          getLoop() const;

    /* 操作 */
    void send(const void*, size_t);
    void send(const utils::StringPiece&);
    void shutdown(); // NOT thread safe
    void close();
    void setTcpNoDelay(bool);
    void startRead();
    void stopRead();

    void setContext(const std::any& context);
    const std::any& getContext() const;

    /* 设置回调 */
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
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    std::string stateToString() const;

private:
    EventLoop& loop_;
    const std::string name_;

    Socket socket_;
    State state_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    Event event_;

    ConnectionCallback connectionCb_;
    MessageCallback messageCb_;
    WriteCompleteCallback writeCompleteCb_;
    HighWaterMarkCallback highWaterMarkCb_;
    CloseCallback closeCb_;
    size_t highWaterMark_;

    utils::Buffer inputBuffer_;
    utils::Buffer outputBuffer_;
    std::any context_;
};

} /* namespace esynet */