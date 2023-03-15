#include "net/TcpClient.h"

#include "net/base/Connector.h"
#include <memory>

using esynet::TcpClient;
using esynet::Reactor;

TcpClient::TcpClient(Reactor& reactor,
                    InetAddress addr,
                    utils::StringPiece name) :
                    reactor_(reactor),
                    name_(name.asString()),
                    retry_(false),
                    tryToConnect_(true),
                    nextConnId_(1) {
    connector_ = std::make_unique<Connector>(reactor_, addr);
    connectionCb_ = TcpConnection::defaultConnectionCallback;
    messageCb_ = TcpConnection::defaultMessageCallback;

    connector_->setConnectionCallback([this](Socket socket) {
        onConnection(socket);
    });
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    bool unique = connection_.unique();
    conn = connection_;
    if(conn) {
        auto cb = [this](TcpConnection& conn) {
            removeConnection(conn);
        };
        reactor_.run([&conn, cb] {
            conn->setCloseCallback(cb);
        });
        if (unique) {
            conn->close();
        }
    }
}

void TcpClient::connect() {
    tryToConnect_ = true;
    connector_->start();
}
void TcpClient::disconnect() {
    tryToConnect_ = false;
    if(connection_) connection_->close();
}
void TcpClient::stop() {
    tryToConnect_ = false;
    connector_->stop();
}

TcpClient::TcpConnectionPtr TcpClient::connection() const {
    return connection_;
}
Reactor& TcpClient::reactor() const {
    return reactor_;
}
const std::string& TcpClient::name() const {
    return name_;
}

void TcpClient::onConnection(Socket socket) {
    std::optional<InetAddress> peerPkg = InetAddress::getPeerAddr(socket);
    std::optional<InetAddress> localPkg = InetAddress::getLocalAddr(socket);
    if(!peerPkg.has_value()) {
        LOG_ERROR("Failed getPeerAddr(fd: {})", socket.fd());
    } else if(!localPkg.has_value()) {
        LOG_ERROR("Failed getLocalAddr(fd: {})", socket.fd());
    }
    nextConnId_++;
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
        closeCb_(tcpConn);
    });
    connection_ = conn;
    conn->connectComplete();
}

void TcpClient::removeConnection(TcpConnection& conn) {
    connection_.reset();
    conn.disconnectComplete();
    if(retry_ && tryToConnect_) {
        connector_->restart();
    }
}