#pragma once

/* Local headers */
#include "utils/NonCopyable.h"
#include "utils/Buffer.h"
#include "utils/Timestamp.h"
#include "net/EventLoop.h"
#include "net/Event.h"
#include "net/TcpConnection.h"
#include "net/base/Socket.h"

namespace esynet {

class TcpServer : public utils::NonCopyable {
private:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback = ConnectionCallback;
    using WriteCompleteCallback = ConnectionCallback;
    using MessageCallback = std::function<void(const TcpConnection&, utils::Buffer*, utils::Timestamp)>;
    using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

public:
    TcpServer(EventLoop&);

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setHighWaterMarkCallback(const HighWaterMarkCallback&, size_t highWaterMark);
    void setCloseCallback(const CloseCallback&);

private:
    EventLoop& loop_;
    // const std::string ip_;
    // const int port_;
    // const std::string name_;

    // Socket socket_;
    // State state_;
    // const InetAddress localAddr_;
    // const InetAddress peerAddr_;

    // ConnectionCallback connectionCb_;
    // CloseCallback closeCb_;
    // WriteCompleteCallback writeCompleteCb_;
    // MessageCallback messageCb_;
    // HighWaterMarkCallback highWaterMarkCb_;

    // size_t highWaterMark_;
    // utils::Buffer inputBuffer_;
    // utils::Buffer outputBuffer_;
    // std::any context_;
};

} /* namespace esynet */