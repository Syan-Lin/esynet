#include "net/TcpClient.h"

#include "net/Connector.h"
#include <memory>

using esynet::TcpClient;
using esynet::Reactor;

TcpClient::TcpClient(InetAddress addr,
                    utils::StringPiece name,
                    bool useEpoll) :
                    reactor_(useEpoll),
                    name_(name.asString()),
                    retry_(false),
                    tryToConnect_(true) {
    connector_ = std::make_unique<Connector>(reactor_, addr);
    connectionCb_    = TcpConnection::defaultConnectionCallback;
    messageCb_       = TcpConnection::defaultMessageCallback;
    closeCb_         = TcpConnection::defaultCloseCallback;
    errorCb_         = TcpConnection::defaultErrorCallback;
    writeCompleteCb_ = TcpConnection::defaultWriteCompleteCallback;
    connector_->setConnectCallback([this](Socket socket) {
        onConnection(socket);
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
Reactor&                    TcpClient::reactor()          { return reactor_; }

void TcpClient::connect() {
    tryToConnect_ = true;
    connector_->start();
}
void TcpClient::disconnect() {
    tryToConnect_ = false;
    if(connection_) connection_->forceClose();
}
void TcpClient::stop() {
    tryToConnect_ = false;
    connector_->stop();
}

void TcpClient::onConnection(Socket socket) {
    std::optional<InetAddress> peerPkg = InetAddress::getPeerAddr(socket);
    std::optional<InetAddress> localPkg = InetAddress::getLocalAddr(socket);
    if(!peerPkg.has_value()) {
        LOG_ERROR("Failed getPeerAddr(fd: {})", socket.fd());
    } else if(!localPkg.has_value()) {
        LOG_ERROR("Failed getLocalAddr(fd: {})", socket.fd());
    }
    InetAddress peer = peerPkg.value();
    InetAddress local = localPkg.value();
    std::string name = name_ + peer.ip() + std::to_string(peer.port());
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(reactor_,
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
    connection_.reset();
    conn.disconnectComplete();
    if(retry_ && tryToConnect_) {
        connector_->restart();
    }
    closeCb_(conn);
}