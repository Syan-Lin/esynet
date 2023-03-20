#include "net/TcpConnection.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/NetAddress.h"
#include "net/base/Looper.h"
#include "exception/SocketException.h"

using esynet::TcpConnection;
using esynet::NetAddress;
using esynet::Looper;
using std::optional;
using TcpInfo = esynet::Socket::TcpInfo;

void TcpConnection::defaultConnectionCallback(TcpConnection& conn) {
    LOG_INFO("Connection from {}:{}", conn.peerAddress().ip(), conn.peerAddress().port());
}
void TcpConnection::defaultMessageCallback(TcpConnection& conn, utils::Buffer& buf, utils::Timestamp) {
    LOG_INFO("Message from {} is \"{}\"", conn.peerAddress().ip(), buf.retrieveAllAsString());
}
void TcpConnection::defaultCloseCallback(TcpConnection& conn) {
    LOG_INFO("Disconnection {}:{}", conn.peerAddress().ip(), conn.peerAddress().port());
}
void TcpConnection::defaultErrorCallback(TcpConnection& conn) {
    LOG_INFO("Error from {}:{}", conn.peerAddress().ip(), conn.peerAddress().port());
    conn.forceClose();
}
void TcpConnection::defaultWriteCompleteCallback(TcpConnection& conn) {
    LOG_INFO("Write complete to {}", conn.peerAddress().ip());
}
void TcpConnection::defaultHighWaterMarkCallback(TcpConnection& conn, size_t size) {
    LOG_WARN("{} reaches high water mark {}", conn.peerAddress().ip(), size);
}

TcpConnection::TcpConnection(Looper& looper,
                            utils::StringPiece name,
                            Socket sock,
                            const NetAddress& localAddr,
                            const NetAddress& peerAddr):
                            looper_(looper),
                            name_(name.asString()),
                            socket_(sock),
                            localAddr_(localAddr),
                            peerAddr_(peerAddr),
                            event_(looper, sock.fd()) {
    connectionCb_    = std::bind(&TcpConnection::defaultConnectionCallback, std::placeholders::_1);
    messageCb_       = std::bind(&TcpConnection::defaultMessageCallback, std::placeholders::_1,
                                                                         std::placeholders::_2,
                                                                         std::placeholders::_3);
    writeCompleteCb_ = std::bind(&TcpConnection::defaultWriteCompleteCallback, std::placeholders::_1);
    highWaterMarkCb_ = std::bind(&TcpConnection::defaultHighWaterMarkCallback, std::placeholders::_1,
                                                                               std::placeholders::_2);
    closeCb_         = std::bind(&TcpConnection::defaultCloseCallback, std::placeholders::_1);
    errorCb_         = std::bind(&TcpConnection::defaultErrorCallback, std::placeholders::_1);

    event_.setReadCallback([this] {
        handleRead();
    });
    event_.setWriteCallback([this] {
        handleWrite();
    });
    event_.setCloseCallback([this] {
        handleClose();
    });
    event_.setErrorCallback([this] {
        errorCb_(*this);
    });
    socket_.setKeepAlive(true);
}
TcpConnection::~TcpConnection() {}

const std::string& TcpConnection::name()         const { return name_; }
Looper&            TcpConnection::looper()       const { return looper_; }
optional<TcpInfo>  TcpConnection::tcpInfo()      const { return socket_.getTcpInfo(); }
bool               TcpConnection::connected()    const { return state_ == kConnected; }
std::string        TcpConnection::tcpInfoStr()   const { return socket_.getTcpInfoString(); }
const NetAddress&  TcpConnection::peerAddress()  const { return peerAddr_; }
const NetAddress&  TcpConnection::localAddress() const { return localAddr_; }
bool               TcpConnection::disconnected() const { return state_ == kDisconnected; }
const std::any&    TcpConnection::getContext()   const { return context_; }

void TcpConnection::setTcpNoDelay(bool on) {
    looper_.run([this, on]{
        socket_.setTcpNoDelay(on);
    });
}
void TcpConnection::setContext(const std::any& context) {
    looper_.run([this, &context] {
        context_ = context;
    });
}
void TcpConnection::setConnectionCallback(const ConnectionCallback& cb) {
    looper_.run([this, &cb] {
        connectionCb_ = std::move(cb);
    });
}
void TcpConnection::setMessageCallback(const MessageCallback& cb) {
    looper_.run([this, &cb] {
        messageCb_ = std::move(cb);
    });
}
void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    looper_.run([this, &cb] {
        writeCompleteCb_ = std::move(cb);
    });
}
void TcpConnection::setCloseCallback(const CloseCallback& cb) {
    looper_.run([this, &cb] {
        closeCb_ = std::move(cb);
    });
}
void TcpConnection::setErrorCallback(const ErrorCallback& cb) {
    looper_.run([this, &cb] {
        errorCb_ = std::move(cb);
    });
}
void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t mark) {
    looper_.run([this, &cb, mark] {
        highWaterMarkCb_ = std::move(cb);
        highWaterMark_   = mark;
    });
}

void TcpConnection::send(const utils::StringPiece msg) {
    send(msg.data(), msg.size());
}
void TcpConnection::send(const void* data, size_t len) {
    if (state_ != kConnected) return;
    LOG_DEBUG("Send {} bytes to {}", len, peerAddress().ip());
    looper_.run([&] {
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
    looper_.run([this] {
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
    looper_.run([this] {
        disconnectComplete();
    });
}
void TcpConnection::forceCloseWithoutCallback() {
    if (state_ == kConnecting || state_ == kDisconnected) return;
    state_ = kDisconnecting;
    looper_.run([this] {
        state_ = kDisconnected;
        event_.cancel();
        socket_.close();
    });
}

void TcpConnection::connectComplete() {
    looper_.assert();

    state_ = kConnected;
    event_.enableRead();
    connectionCb_(*this);
}
void TcpConnection::disconnectComplete() {
    looper_.assert();

    state_ = kDisconnected;
    handleClose();
}

void TcpConnection::handleRead() {
    looper_.assert();

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
    looper_.assert();

    if(event_.writable()) {
        try {
            ssize_t bytes = socket_.write(sendBuffer_.beginRead(), sendBuffer_.readableBytes());
            sendBuffer_.retrieve(bytes);
            if(sendBuffer_.readableBytes() == 0) {
                event_.disableWrite();
                if(writeCompleteCb_) {
                    looper_.queue([this] {
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
void TcpConnection::handleClose() {
    looper_.assert();

    event_.cancel();
    socket_.close();
    closeCb_(*this);
}