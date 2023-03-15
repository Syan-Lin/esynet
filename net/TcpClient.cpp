#include "net/TcpClient.h"

#include "net/base/Connector.h"
#include <memory>

using esynet::TcpClient;

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
    bool unique = false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn) {
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
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(connection_) connection_->shutdown();
    }
}

void TcpClient::stop() {
    tryToConnect_ = false;
    connector_->stop();
}

void TcpClient::onConnection(Socket socket) {
    std::optional<InetAddress> peerPkg = InetAddress::getPeerAddr(socket);
    std::optional<InetAddress> localPkg = InetAddress::getLocalAddr(socket);
    if(!peerPkg.has_value() || !localPkg.has_value()) {

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
    conn->setCloseCallback([this](TcpConnection& tcpConn) {
        removeConnection(tcpConn);
    });
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(TcpConnection& conn) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_.reset();
    }
    reactor_.queue([&conn] {
        conn.connectDestroyed();
    });
    if(retry_ && tryToConnect_) {
        connector_->restart();
    }
}