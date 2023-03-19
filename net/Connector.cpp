#include "net/Connector.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/Looper.h"
#include "net/base/Event.h"
#include "utils/ErrorInfo.h"
#include "exception/NetworkException.h"

using esynet::Connector;
const int Connector::kMaxRetryDelayMs  = 30 * 1000;
const int Connector::kInitRetryDelayMs = 500;

Connector::Connector(Looper& looper, const NetAddress& serverAddr):
                    looper_(looper),
                    serverAddr_(serverAddr),
                    couldConnect(true),
                    state_(kDisconnected),
                    retryDelayMs_(kInitRetryDelayMs) {}

Connector::~Connector() {}

void Connector::setConnectCallback(ConnectCallback cb) {
    connectCb_ = std::move(cb);
}

void Connector::start() {
    looper_.assert();

    if(!couldConnect) return;
    couldConnect = false;

    Socket socket;
    try {
        socket.connect(serverAddr_);
        checkConnect(socket);
    } catch(exception::NetworkException& e) {
        switch (e.err()) {
            case EINPROGRESS: case EINTR: case EISCONN:
                checkConnect(socket);
                break;
            case EAGAIN: case EADDRINUSE: case EADDRNOTAVAIL:
            case ECONNREFUSED: case ENETUNREACH:
                retry(socket);
                break;
            case EACCES: case EPERM: case EAFNOSUPPORT:
            default:
                throw e;
                break;
        }
    }
}

void Connector::stop() {
    looper_.assert();

    if(!couldConnect) return;
    couldConnect = true;

    if(event_) event_->cancel();
    state_ = kDisconnected;
}

void Connector::restart() {
    looper_.assert();

    couldConnect = true;
    state_ = kDisconnected;
    retryDelayMs_ = kInitRetryDelayMs;
    start();
}

void Connector::retry(Socket socket) {
    looper_.assert();

    couldConnect = true;
    socket.close();
    state_ = kDisconnected;
    looper_.runAfter(retryDelayMs_, [this] {
        start();
    });
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
}

void Connector::checkConnect(Socket socket) {
    looper_.assert();

    state_ = kConnecting;
    event_ = std::make_unique<Event>(looper_, socket.fd());

    event_->setWriteCallback([this, socket] {
        if(state_ != kConnecting) return;
        // 注销监听可写事件：
        // 接下来要么将监听权交由TcpConnection
        // 要么重新创建套接字进行尝试
        // 该回调一个套接字至多执行一次
        event_->cancel();
        // 可写表示连接完成，但不一定成功，仍然需要检查
        std::optional<NetAddress> peer = NetAddress::getPeerAddr(socket);
        if(peer.has_value()) {
            onConnect(socket, peer.value());
        } else {
            LOG_WARN("Unable to connect to the destination address, will retry");
            retry(socket);
        }
    });
    event_->setErrorCallback([this, socket] {
        if (state_ == kConnecting) {
            auto err = Socket::getSocketError(socket);
            if(err.has_value()) {
                LOG_WARN("Connection error(err: {}) will retry", errnoStr(err.value()));
            }
            retry(socket);
        }
    });
    event_->enableWrite();
}

void Connector::onConnect(Socket socket, NetAddress peer) {
    looper_.assert();

    state_ = kConnected;
    connectCb_(socket, peer);
}