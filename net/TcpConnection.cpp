#include "net/TcpConnection.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/InetAddress.h"
#include "net/base/Reactor.h"
#include "exception/SocketException.h"

using esynet::TcpConnection;
using esynet::InetAddress;
using esynet::Reactor;
using std::optional;
using TcpInfo = esynet::Socket::TcpInfo;
using esynet::utils::Buffer;

void TcpConnection::defaultConnectionCallback(TcpConnection& conn) {
    LOG_DEBUG("Connection from {}:{}", conn.peerAddress().ip());
}
void TcpConnection::defaultMessageCallback(TcpConnection& conn, utils::Buffer& buf, utils::Timestamp) {
    LOG_DEBUG("Message from {} is \"{}\"", conn.peerAddress().ip(), buf.retrieveAllAsString());
}
void TcpConnection::defaultCloseCallback(TcpConnection& conn) {
    LOG_DEBUG("Disconnection {}", conn.peerAddress().ip());
}
void TcpConnection::defaultErrorCallback(TcpConnection& conn) {
    LOG_DEBUG("Error from {}", conn.peerAddress().ip());
}
void TcpConnection::defaultWriteCompleteCallback(TcpConnection& conn) {
    LOG_DEBUG("Write complete to {}", conn.peerAddress().ip());
}
void TcpConnection::defaultHighWaterMarkCallback(TcpConnection& conn, size_t) {
    LOG_DEBUG("{} reaches high water mark", conn.peerAddress().ip());
}

TcpConnection::TcpConnection(Reactor& reactor,
                            utils::StringPiece name,
                            Socket sock,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr):
                            reactor_(reactor),
                            name_(name.asString()),
                            socket_(sock),
                            localAddr_(localAddr),
                            peerAddr_(peerAddr),
                            event_(reactor, sock.fd()) {
    state_ = kConnecting;
    highWaterMark_ = 64_MB;
    connectionCb_    = std::bind(&TcpConnection::defaultConnectionCallback, std::placeholders::_1);
    messageCb_       = std::bind(&TcpConnection::defaultMessageCallback, std::placeholders::_1,
                                                                         std::placeholders::_2,
                                                                         std::placeholders::_3);
    writeCompleteCb_ = std::bind(&TcpConnection::defaultWriteCompleteCallback, std::placeholders::_1);
    highWaterMarkCb_ = std::bind(&TcpConnection::defaultHighWaterMarkCallback, std::placeholders::_1,
                                                                               std::placeholders::_2);
    closeCb_         = std::bind(&TcpConnection::defaultCloseCallback, std::placeholders::_1);
    errorCb_         = std::bind(&TcpConnection::defaultErrorCallback, std::placeholders::_1);

    event_.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    event_.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    event_.setCloseCallback(std::bind(&TcpConnection::disconnectComplete, this));
    event_.setErrorCallback([this] {
        errorCb_(*this);
    });
    socket_.setKeepAlive(true);
}
TcpConnection::~TcpConnection() {}

const std::string& TcpConnection::name()         const { return name_; }
Reactor&           TcpConnection::reactor()      const { return reactor_; }
optional<TcpInfo>  TcpConnection::tcpInfo()      const { return socket_.getTcpInfo(); }
bool               TcpConnection::connected()    const { return state_ == kConnected; }
std::string        TcpConnection::tcpInfoStr()   const { return socket_.getTcpInfoString(); }
const InetAddress& TcpConnection::peerAddress()  const { return peerAddr_; }
const InetAddress& TcpConnection::localAddress() const { return localAddr_; }
bool               TcpConnection::disconnected() const { return state_ == kDisconnected; }
const std::any&    TcpConnection::getContext()   const { return context_; }
Buffer&            TcpConnection::readBuffer() { return readBuffer_; }
Buffer&            TcpConnection::sendBuffer() { return sendBuffer_; }

void TcpConnection::setTcpNoDelay(bool on)                                    { socket_.setTcpNoDelay(on); }
void TcpConnection::setContext(const std::any& context)                       { context_ = context; }
void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)       { connectionCb_    = cb; }
void TcpConnection::setMessageCallback(const MessageCallback& cb)             { messageCb_       = cb; }
void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCb_ = cb; }
void TcpConnection::setCloseCallback(const CloseCallback& cb)                 { closeCb_         = cb; }
void TcpConnection::setErrorCallback(const ErrorCallback& cb)                 { errorCb_         = cb; }
void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t mark) {
    highWaterMarkCb_ = cb;
    highWaterMark_   = mark;
}

void TcpConnection::send(const utils::StringPiece msg) {
    send(msg.data(), msg.size());
}
void TcpConnection::send(const void* data, size_t len) {
    if (state_ != kConnected) return;
    reactor_.run([&] {
        size_t wrote = 0;
        bool error = false;

        bool isFirstSend = !event_.writable() && sendBuffer_.readableBytes() == 0;
        if(isFirstSend) {
            try {
                wrote = socket_.write(data, len);
                len -= wrote;
                if(len == 0 && writeCompleteCb_) {
                    writeCompleteCb_(*this);
                }
            } catch(exception::SocketException& e) {
                if(errno != EWOULDBLOCK && (errno == EPIPE || errno == ECONNRESET)) {
                    LOG_ERROR("{}", e.detail());
                    error = true;
                }
            }
        }

        if(!error && len > 0) {
            size_t dataInBuffer = sendBuffer_.readableBytes();
            if(dataInBuffer + len >= highWaterMark_ && highWaterMarkCb_) {
                highWaterMarkCb_(*this, sendBuffer_.readableBytes());
            }
            sendBuffer_.append(static_cast<const char*>(data) + wrote, len);
            if(!event_.writable()) {
                event_.enableWrite();
            }
        }
    });
}

void TcpConnection::shutdown() {
    if (state_ != kConnected) return;
    state_ = kDisconnecting;
    reactor_.run([this] {
        if(event_.writable()) {
            LOG_WARN("Try to shutdown while data has not been send");
            return;
        }
        socket_.shutdownWrite();
    });
}
void TcpConnection::forceClose() {
    if (state_ == kConnecting || state_ == kDisconnected) return;
    state_ = kDisconnecting;
    reactor_.run([this] {
        disconnectComplete();
    });
}
void TcpConnection::enableRead() {
    reactor_.run([this] {
        if(event_.readable()) return;
        event_.enableRead();
    });
}
void TcpConnection::disableRead() {
    reactor_.run([this] {
        if(!event_.readable()) {
            LOG_WARN("Try to disable read while data has not been read");
            return;
        }
        event_.disableRead();
    });
}

void TcpConnection::connectComplete() {
    state_ = kConnected;
    event_.enableRead();
    connectionCb_(*this);
}
void TcpConnection::disconnectComplete() {
    state_ = kDisconnected;
    event_.disableAll();
    reactor_.removeEvent(event_);
    closeCb_(*this);
}

void TcpConnection::handleRead() {
    reactor_.assert();

    try {
        ssize_t bytes = readBuffer_.readSocket(socket_);
        if(bytes > 0) {
            messageCb_(*this, readBuffer_, utils::Timestamp::now());
        } else {
            disconnectComplete();
        }
    } catch(exception::SocketException& e) {
        LOG_ERROR("{}", e.detail());
        errorCb_(*this);
    }
}
// 当send函数一次发不完时，会注册监听可写事件，在可写时执行该函数继续发送
void TcpConnection::handleWrite() {
    reactor_.assert();

    if(event_.writable()) {
        try {
            ssize_t bytes = socket_.write(sendBuffer_.beginRead(), sendBuffer_.readableBytes());
            sendBuffer_.retrieve(bytes);
            if(sendBuffer_.readableBytes() == 0) {
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
        } catch(exception::SocketException& e) {
            LOG_ERROR("{}", e.detail());
        }
    } else {
        LOG_ERROR("TcpConnection::handleWrite() can't write");
    }
}