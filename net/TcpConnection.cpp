#include "net/TcpConnection.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/InetAddress.h"
#include "net/Reactor.h"

using esynet::TcpConnection;
using esynet::InetAddress;
using esynet::Reactor;

void TcpConnection::defaultConnectionCallback(TcpConnection& conn) {
    LOG_DEBUG("Connection from {} is {}", conn.peerAddress().ip(),
                                        (conn.connected() ? "UP" : "DOWN"));
}

void TcpConnection::defaultMessageCallback(
                                    TcpConnection& conn,
                                    utils::Buffer& buf,
                                    utils::Timestamp) {
    LOG_DEBUG("Message from {} is \"{}\"",
        conn.peerAddress().ip(), buf.retrieveAllAsString());
}

TcpConnection::TcpConnection(Reactor& reactor,
                            utils::StringPiece name,
                            Socket sock,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr):
                            reactor_(reactor),
                            name_(name.asString()),
                            state_(kConnecting),
                            socket_(sock),
                            event_(reactor, sock.fd()),
                            localAddr_(localAddr),
                            peerAddr_(peerAddr),
                            highWaterMark_(64_MB) {
    event_.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    event_.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    event_.setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    event_.setErrorCallback(std::bind(&TcpConnection::handleError, this));
    socket_.setKeepAlive(true);
}

TcpConnection::~TcpConnection() {}

const std::string& TcpConnection::name() const {
    return name_;
}
const InetAddress& TcpConnection::localAddress() const {
    return localAddr_;
}
const InetAddress& TcpConnection::peerAddress() const {
    return peerAddr_;
}
bool TcpConnection::connected() const {
    return state_ == kConnected;
}
bool TcpConnection::disconnected() const {
    return state_ == kDisconnected;
}
std::optional<TcpConnection::TcpInfo> TcpConnection::tcpInfo() const {
    return socket_.getTcpInfo();
}
std::string TcpConnection::tcpInfoStr() const {
    return socket_.getTcpInfoString();
}
Reactor& TcpConnection::reactor() const {
    return reactor_;
}

void TcpConnection::send(const void* data, size_t len) {
    if (state_ != kConnected) return;
    reactor_.run([&] {
        size_t wrote = 0;
        bool fault = false;

        // 如果不在写状态，且输出缓冲区为空，则直接写入
        if(!event_.writable() && outputBuffer_.readableBytes() == 0) {
            wrote = socket_.write(data, len);
            if(wrote >= 0) {
                len -= wrote;
                // 完成写入，将回调入列
                if(len == 0 && writeCompleteCb_) {
                    reactor_.queue([this] {
                        writeCompleteCb_(*this);
                    });
                }
            } else {
                // 写入出错
                wrote = 0;
                if(errno != EWOULDBLOCK &&
                        (errno == EPIPE || errno == ECONNRESET)) {
                    LOG_ERROR("sendImplement failed(fd: {}, errno: {})", socket_.fd(), strerror(errno));
                    fault = true;
                }
            }
        }

        if(!fault && len > 0) {
            size_t remain = outputBuffer_.readableBytes();
            if(remain + len >= highWaterMark_ && highWaterMarkCb_) {
                // 囤积数据过多
                reactor_.queue([this] {
                    highWaterMarkCb_(*this, outputBuffer_.readableBytes());
                });
            }
            outputBuffer_.append(static_cast<const char*>(data) + wrote, len);
            if(!event_.writable()) {
                event_.enableWrite();
            }
        }
    });
}
void TcpConnection::send(const utils::StringPiece msg) {
    send(msg.data(), msg.size());
}
void TcpConnection::shutdown() {
    if (state_ != kConnected) return;
    state_ = kDisconnecting;
    reactor_.run([this] {
        if(!event_.writable()) {
            socket_.shutdownWrite();
        }
    });
}
void TcpConnection::close() {
    if (state_ == kConnecting || state_ == kDisconnected) return;
    state_ = kDisconnecting;
    reactor_.run([this] {
        handleClose();
    });
}
void TcpConnection::setTcpNoDelay(bool on) {
    socket_.setTcpNoDelay(on);
}
void TcpConnection::startRead() {
    reactor_.run([this] {
        if(!event_.readable()) {
            event_.enableRead();
        }
    });
}
void TcpConnection::stopRead() {
    reactor_.run([this] {
        if(event_.readable()) {
            event_.disableRead();
        }
    });
}

void TcpConnection::setContext(const std::any& context) {
    context_ = context;
}
const std::any& TcpConnection::getContext() const {
    return context_;
}

void TcpConnection::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCb_ = cb;
}
void TcpConnection::setMessageCallback(const MessageCallback& cb) {
    messageCb_ = cb;
}
void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCb_ = cb;
}
void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t mark) {
    highWaterMarkCb_ = cb;
    highWaterMark_ = mark;
}
void TcpConnection::setCloseCallback(const CloseCallback& cb) {
    closeCb_ = cb;
}

esynet::utils::Buffer& TcpConnection::inputBuffer() {
    return inputBuffer_;
}
esynet::utils::Buffer& TcpConnection::outputBuffer() {
    return outputBuffer_;
}

void TcpConnection::connectEstablished() {
    state_ = kConnected;
    event_.enableRead();
    connectionCb_(*this);
}
void TcpConnection::connectDestroyed() {
    state_ = kDisconnected;
    event_.disableAll();
    connectionCb_(*this);
    reactor_.removeEvent(event_);
}

void TcpConnection::handleRead() {
    if(!reactor_.isInLoopThread()) {
        LOG_FATAL("Try to handleRead in another thread(reactor({:p}))", static_cast<void*>(this));
    }
    int savedErrno = 0;
    ssize_t bytes = inputBuffer_.readSocket(socket_, savedErrno);
    if(bytes > 0) {
        messageCb_(*this, inputBuffer_, utils::Timestamp::now());
    } else if(bytes == 0) {
        handleClose();
    } else {
        LOG_ERROR("TcpConnection::handleRead() error: {}", strerror(savedErrno));
        handleError();
    }
}
// 当send函数一次发不完时，会注册监听可写事件，在可写时执行该函数继续发送
void TcpConnection::handleWrite() {
    if(!reactor_.isInLoopThread()) {
        LOG_FATAL("Try to handleWrite in another thread(reactor({:p}))", static_cast<void*>(this));
    }
    if(event_.writable()) {
        ssize_t bytes = socket_.write(outputBuffer_.beginRead(), outputBuffer_.readableBytes());
        if(bytes > 0) {
            outputBuffer_.retrieve(bytes);
            if(outputBuffer_.readableBytes() == 0) {
                event_.disableWrite();
                if(writeCompleteCb_) {
                    reactor_.queue([this] {
                        writeCompleteCb_(*this);
                    });
                }
                if(state_ == kDisconnecting) {
                    shutdown();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite() write error: {}", strerror(errno));
        }
    } else {
        LOG_ERROR("TcpConnection::handleWrite() can't write");
    }
}
void TcpConnection::handleClose() {
    event_.disableAll();
    closeCb_(*this);
}
void TcpConnection::handleError() {
    LOG_ERROR("TcpConnection::handleError() error: {}", strerror(1));
}