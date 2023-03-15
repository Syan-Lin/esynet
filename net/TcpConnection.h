#pragma once

/* Standard headers */
#include <any>
#include <memory>

/* Local headers */
#include "net/Event.h"
#include "net/base/Socket.h"
#include "utils/Buffer.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "utils/Timestamp.h"
#include "net/base/InetAddress.h"

namespace esynet {

class Reactor;

/* 该类是直接和业务代码接触的类，很大可能发生跨线程调用
 * 所以该类中的大部分操作都委托给Reactor */
class TcpConnection : utils::NonCopyable {
private:
    using TcpInfo = Socket::TcpInfo;
    enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };

public:
    using TcpConnectionPtr      = std::shared_ptr<TcpConnection>;
    using ConnectionCallback    = std::function<void(TcpConnection&)>;
    using MessageCallback       = std::function<void(TcpConnection&, utils::Buffer&, utils::Timestamp)>;
    using HighWaterMarkCallback = std::function<void(TcpConnection&, size_t)>;
    using CloseCallback         = ConnectionCallback;
    using WriteCompleteCallback = ConnectionCallback;
    using ErrorCallback         = ConnectionCallback;

public:
    static void defaultConnectionCallback(TcpConnection&);
    static void defaultCloseCallback(TcpConnection&);
    static void defaultErrorCallback(TcpConnection&);
    static void defaultWriteCompleteCallback(TcpConnection&);
    static void defaultMessageCallback(TcpConnection&, utils::Buffer&, utils::Timestamp);
    static void defaultHighWaterMarkCallback(TcpConnection&, size_t);

public:
    TcpConnection(Reactor&, utils::StringPiece, Socket,
                    const InetAddress& local, const InetAddress& peer);
    ~TcpConnection();

    /* 状态相关，线程安全 */
    const std::string&      name()          const;
    const InetAddress&      localAddress()  const;
    const InetAddress&      peerAddress()   const;
    bool                    connected()     const;
    bool                    disconnected()  const;
    std::optional<TcpInfo>  tcpInfo()       const;
    std::string             tcpInfoStr()    const;
    Reactor&                reactor()       const;

    /* 操作，非线程安全 */
    void send(const void*, size_t);
    void send(const utils::StringPiece);
    void shutdown();
    void close();
    void setTcpNoDelay(bool);
    void enableRead();
    void disableRead();
    void setContext(const std::any&);
    const std::any& getContext() const;

    /* 设置回调 */
    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setHighWaterMarkCallback(const HighWaterMarkCallback&, size_t highWaterMark);
    void setCloseCallback(const CloseCallback&);
    void setErrorCallback(const ErrorCallback&);

    utils::Buffer& inputBuffer();
    utils::Buffer& outputBuffer();

    /* 当连接建立完成时调用 */
    void connectComplete();
    /* 当连接断开完成时调用 */
    void disconnectComplete();

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    std::string stateToString() const;

private:
    Reactor& reactor_;
    const std::string name_;

    Socket socket_;
    std::atomic<State> state_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    Event event_;

    ConnectionCallback connectionCb_;
    MessageCallback messageCb_;
    WriteCompleteCallback writeCompleteCb_;
    HighWaterMarkCallback highWaterMarkCb_;
    CloseCallback closeCb_;
    ErrorCallback errorCb_;
    size_t highWaterMark_;

    utils::Buffer inputBuffer_;
    utils::Buffer outputBuffer_;
    std::any context_;
};

} /* namespace esynet */