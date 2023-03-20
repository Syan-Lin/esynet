#pragma once

/* Standard headers */
#include <any>
#include <memory>

/* Local headers */
#include "net/base/Event.h"
#include "net/base/Socket.h"
#include "utils/Buffer.h"
#include "utils/NonCopyable.h"
#include "utils/StringPiece.h"
#include "utils/Timestamp.h"
#include "net/base/NetAddress.h"

namespace esynet {

class Looper;

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
    TcpConnection(Looper&, utils::StringPiece, Socket, const NetAddress& local, const NetAddress& peer);
    ~TcpConnection();

    auto name()         const -> const std::string&;
    auto localAddress() const -> const NetAddress&;
    auto peerAddress()  const -> const NetAddress&;
    auto tcpInfo()      const -> std::optional<TcpInfo>;
    auto tcpInfoStr()   const -> std::string;
    auto looper()       const -> Looper&;
    bool connected()    const;
    bool disconnected() const;

    void send(const void*, size_t);
    void send(const utils::StringPiece);
    void shutdown();
    void forceClose();
    void forceCloseWithoutCallback();
    void setTcpNoDelay(bool);

    void setContext(const std::any&);
    auto getContext() const -> const std::any&;

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setHighWaterMarkCallback(const HighWaterMarkCallback&, size_t highWaterMark);
    void setCloseCallback(const CloseCallback&);
    void setErrorCallback(const ErrorCallback&);

    /* 当连接建立完成时调用，非线程安全 */
    void connectComplete();
    /* 当连接断开完成时调用，非线程安全 */
    void disconnectComplete();

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    std::string stateToString() const;

private:
    Looper& looper_;
    const std::string name_;

    Event event_;
    Socket socket_;
    const NetAddress peerAddr_;
    const NetAddress localAddr_;
    std::atomic<State> state_{kConnecting};

    CloseCallback closeCb_;
    ErrorCallback errorCb_;
    MessageCallback messageCb_;
    ConnectionCallback connectionCb_;
    WriteCompleteCallback writeCompleteCb_;
    HighWaterMarkCallback highWaterMarkCb_;

    size_t highWaterMark_{64_MB};

    std::any context_;
    utils::Buffer readBuffer_;
    utils::Buffer sendBuffer_;
};

} /* namespace esynet */