#include "net/base/Connector.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/Reactor.h"
#include "net/Event.h"
#include "utils/ErrorInfo.h"
#include "exception/SocketException.h"

using esynet::Connector;
const int Connector::kMaxRetryDelayMs;
const int Connector::kInitRetryDelayMs;

Connector::Connector(Reactor& reactor, const InetAddress& serverAddr):
                    reactor_(reactor),
                    serverAddr_(serverAddr),
                    tryToConnect_(false),
                    state_(kDisconnected),
                    retryDelayMs_(kInitRetryDelayMs) {}

Connector::~Connector() {}

void Connector::setConnectionCallback(ConnectionCallback callback) {
    connCb_ = callback;
}

void Connector::start() {
    if(tryToConnect_) return;

    tryToConnect_ = true;
    Socket socket;
    try {
        socket.connect(serverAddr_);
        checkConnect(socket);
    } catch(exception::SocketException& e) {
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
    if(!tryToConnect_) return;

    tryToConnect_ = false;
    if(event_) event_->disableAll();
    event_.reset();
    state_ = kDisconnected;
}

void Connector::restart() {
    state_ = kDisconnected;
    retryDelayMs_ = kInitRetryDelayMs;
    tryToConnect_ = true;
    start();
}

void Connector::retry(Socket socket) {
    socket.close();
    state_ = kDisconnected;
    if(tryToConnect_) {
        reactor_.runAfter(retryDelayMs_, [this] {
            start();
        });
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
}

void Connector::checkConnect(Socket socket) {
    state_ = kConnecting;
    event_ = std::make_unique<Event>(reactor_, socket.fd());

    event_->setWriteCallback([this, socket] {
        if(state_ != kConnecting) return;
        /* 注销监听可写事件：
            * 接下来要么将监听权交由TcpConnection
            * 要么重新创建套接字进行尝试
            * 该回调一个套接字至多执行一次 */
        event_->disableAll();
        event_.reset();
        // 可写表示连接完成，但不一定成功，仍然需要检查
        auto err = Socket::getSocketError(socket);
        if(err.has_value()) {
            LOG_WARN("Connection error(err: {}) will retry", errnoStr(err.value()));
            retry(socket);
        } else if (Socket::isSelfConnect(socket)) {
            LOG_ERROR("Connection self connect(err: {})", errnoStr(err.value()));
        } else {
            // 连接成功
            onConnection(socket);
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

void Connector::onConnection(Socket socket) {
    state_ = kConnected;
    connCb_(socket);
}