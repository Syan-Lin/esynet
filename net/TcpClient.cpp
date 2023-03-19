#include "net/TcpClient.h"

#include "net/Connector.h"
#include <memory>

using esynet::TcpClient;
using esynet::Looper;

TcpClient::TcpClient(NetAddress addr,
                    utils::StringPiece name,
                    bool useEpoll) :
                    looper_(useEpoll),
                    name_(name.asString()) {
    connector_ = std::make_unique<Connector>(looper_, addr);
    connectionCb_    = TcpConnection::defaultConnectionCallback;
    messageCb_       = TcpConnection::defaultMessageCallback;
    closeCb_         = TcpConnection::defaultCloseCallback;
    errorCb_         = TcpConnection::defaultErrorCallback;
    writeCompleteCb_ = TcpConnection::defaultWriteCompleteCallback;
    connector_->setConnectCallback([this](Socket socket, NetAddress peer) {
        onConnection(socket, peer);
    });
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    if(connection_ && connection_.unique()) {
        connection_->forceClose();
    }
}

TcpClient::TcpConnectionPtr TcpClient::connection() const { return connection_; }
const std::string&          TcpClient::name()       const { return name_; }
Looper&                    TcpClient::looper()          { return looper_; }

void TcpClient::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCb_ = std::move(cb);
}
void TcpClient::setMessageCallback(const MessageCallback& cb) {
    messageCb_ = std::move(cb);
}
void TcpClient::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCb_ = std::move(cb);
}
void TcpClient::setCloseCallback(const CloseCallback& cb) {
    closeCb_ = std::move(cb);
}
void TcpClient::setErrorCallback(const ErrorCallback& cb) {
    errorCb_ = std::move(cb);
}

void TcpClient::connect() {
    looper_.assert();

    connector_->start();
}
void TcpClient::disconnect() {
    looper_.assert();

    if(connection_) connection_->forceClose();
}
void TcpClient::reconnect() {
    looper_.assert();

    connector_->restart();
}
void TcpClient::stopConnecting() {
    looper_.assert();

    connector_->stop();
}

void TcpClient::start() {
    looper_.assert();

    looper_.start();
}
void TcpClient::shutdown() {
    looper_.assert();

    looper_.stop();
}

void TcpClient::onConnection(Socket socket, NetAddress peer) {
    looper_.assert();

    std::optional<NetAddress> localPkg = NetAddress::getLocalAddr(socket);
    NetAddress local;
    if(!localPkg.has_value()) {
        LOG_ERROR("Failed getLocalAddr(fd: {})", socket.fd());
    } else {
        local = localPkg.value();
    }
    std::string name = name_ + peer.ip() + std::to_string(peer.port());
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(looper_,
                                                            name,
                                                            socket,
                                                            local,
                                                            peer);
    conn->setConnectionCallback(connectionCb_);
    conn->setMessageCallback(messageCb_);
    conn->setWriteCompleteCallback(writeCompleteCb_);
    conn->setErrorCallback(errorCb_);
    conn->setCloseCallback([this](TcpConnection& tcpConn) {
        removeConnection(tcpConn);
    });
    conn->connectComplete();
    connection_ = conn;
}

void TcpClient::removeConnection(TcpConnection& conn) {
    looper_.assert();

    connection_.reset();
    closeCb_(conn);
}