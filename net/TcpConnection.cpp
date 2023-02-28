#include "net/TcpConnection.h"
#include "logger/Logger.h"
#include "net/base/InetAddress.h"

using esynet::TcpConnection;
using esynet::InetAddress;
using esynet::EventLoop;

void TcpConnection::defaultConnectionCallback(const TcpConnection& ptr) {
    LOG_INFO("Connection from {} is {}", ptr.peerAddress().ip(),
                                        (ptr.connected() ? "UP" : "DOWN"));
}

void TcpConnection::defaultMessageCallback(
        const TcpConnection& conn, utils::Buffer& buf, utils::Timestamp) {
    buf.retrieveAll();
}

TcpConnection::TcpConnection(EventLoop& loop,
                            utils::StringPiece name,
                            Socket sock,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr):
                            loop_(loop),
                            name_(name.asString()),
                            state_(kConnecting),
                            socket_(sock),
                            event_(loop, sock.fd()),
                            localAddr_(localAddr),
                            peerAddr_(peerAddr),
                            highWaterMark_(64_MB) {
    event_.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    event_.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    event_.setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    event_.setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG("TcpConnection::ctor[{}] at {:p} fd={}", name_, this, sock.fd());
    socket_.setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG("TcpConnection::dtor[{}] at {:p} fd={}", name_, this, socket_.fd());
}

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
TcpConnection::TcpInfo TcpConnection::getTcpInfo() const {
    return socket_.getTcpInfo();
}
std::string TcpConnection::getTcpInfoString() const {
    return socket_.getTcpInfoString();
}
EventLoop& TcpConnection::getLoop() const {
    return loop_;
}

void TcpConnection::send(const void* data, size_t len) {
    if (state_ == kConnected) {
        loop_.run([&]{
            size_t wrote = 0;
            bool fault = false;

            if(state_ == kDisconnected) {
                LOG_WARN("Try send message to a disconnected connection");
                return;
            }

            // 如果不在写状态，且输出缓冲区为空，则直接写入
            if(!event_.writable() && outputBuffer_.readableBytes() == 0) {
                wrote = socket_.write(data, len);
                if(wrote >= 0) {
                    len -= wrote;
                    // 完成写入，将回调入列
                    if(len == 0 && writeCompleteCb_) {
                        loop_.queue([this] {
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
                    loop_.queue([this] {
                        highWaterMarkCb_(*this, outputBuffer_.readableBytes());
                    });
                }
                outputBuffer_.append(static_cast<const char*>(data) + wrote, len);
                if(!event_.writable()) {
                    event_.enableWriting();
                }
            }
        });
    }
}
void TcpConnection::send(const utils::StringPiece& msg) {
    send(msg.data(), msg.size());
}
void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        loop_.run([this] {
            if(!event_.writable()) {
                socket_.shutdownWrite();
            }
        });
    }
}
void TcpConnection::close() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        state_ = kDisconnecting;
        loop_.run([this] {
            handleClose();
        });
    }
}
void TcpConnection::setTcpNoDelay(bool on) {
    socket_.setTcpNoDelay(on);
}
void TcpConnection::startRead() {
    loop_.run([this] {
        if(!event_.readable()) {
            event_.enableReading();
        }
    });
}
void TcpConnection::stopRead() {
    loop_.run([this] {
        if(event_.readable()) {
            event_.disableReading();
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
    event_.enableReading();
    connectionCb_(*this);
}
void TcpConnection::connectDestroyed() {
    state_ = kDisconnected;
    event_.disableAll();
    connectionCb_(*this);
    loop_.removeEvent(event_);
}

void TcpConnection::handleRead() {
    if(!loop_.isInLoopThread()) {
        LOG_FATAL("Try to handleRead in another thread(loop({:p}))", static_cast<void*>(this));
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
    if(!loop_.isInLoopThread()) {
        LOG_FATAL("Try to handleWrite in another thread(loop({:p}))", static_cast<void*>(this));
    }
    if(event_.writable()) {
        ssize_t bytes = socket_.write(outputBuffer_.beginRead(), outputBuffer_.readableBytes());
        if(bytes > 0) {
            outputBuffer_.retrieve(bytes);
            if(outputBuffer_.readableBytes() == 0) {
                event_.disableWriting();
                if(writeCompleteCb_) {
                    loop_.queue([this] {
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
    // LOG_ERROR("TcpConnection::handleError() error: {}", strerror(1));
}