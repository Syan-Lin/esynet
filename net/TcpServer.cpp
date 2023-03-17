#include "net/TcpServer.h"

/* Local headers */
#include "net/base/Reactor.h"

/* Standard headers */
#include <functional>

using esynet::Reactor;
using esynet::TcpServer;
using esynet::ReactorThreadPoll;

TcpServer::TcpServer(Reactor& reactor, InetAddress addr, utils::StringPiece name):
        reactor_(reactor),
        port_(addr.port()),
        ip_(addr.ip()),
        name_(name.asString()),
        acceptor_(reactor, addr),
        threadPoll_(reactor),
        started_(false),
        nextConnId_(1) {
    connectionCb_ = TcpConnection::defaultConnectionCallback;
    messageCb_ = TcpConnection::defaultMessageCallback;
    acceptor_.setAcceptCallback(std::bind(
        &TcpServer::onConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for(auto& iter : connections_) {
        iter.second->reactor().run([&iter] {
            iter.second->disconnectComplete();
        });
    }
    connections_.clear();
}

void TcpServer::start() {
    reactor_.assert();
    if(!started_) {
        threadPoll_.start();
        acceptor_.listen();
    }
    started_ = true;
}

const std::string&  TcpServer::ip() const   { return ip_; }
const std::string&  TcpServer::name() const { return name_; }
int                 TcpServer::port() const { return port_; }
Reactor&          TcpServer::reactor()    { return reactor_; }

void TcpServer::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCb_ = cb;
}
void TcpServer::setMessageCallback(const MessageCallback& cb) {
    messageCb_ = cb;
}
void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCb_ = cb;
}
void TcpServer::setCloseCallback(const CloseCallback& cb) {
    closeCb_ = cb;
}
void TcpServer::setErrorCallback(const ErrorCallback& cb) {
    errorCb_ = cb;
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback& cb) {
    threadPoll_.setInitCallback(cb);
}
void TcpServer::setThreadNumInPool(size_t numThreads) {
    threadPoll_.setThreadNum(numThreads);
}

ReactorThreadPoll& TcpServer::threadPoll() {
    return threadPoll_;
}
void TcpServer::setThreadPollStrategy(Strategy strategy) {
    strategy_ = strategy;
}

void TcpServer::onConnection(Socket socket, const InetAddress& peerAddr) {
    reactor_.assert();
    Reactor* reactor;
    if(strategy_ == kLightest) {
        reactor = threadPoll_.getLightest();
    } else {
        reactor = threadPoll_.getNext();
    }
    std::string connName = name_ + "-" + peerAddr.ip() + ":"
                            + std::to_string(peerAddr.port()) + "-" + std::to_string(nextConnId_++);

    std::optional<InetAddress> local = InetAddress::getLocalAddr(socket);
    InetAddress localAddr;
    if(local.has_value()) {
        localAddr = local.value();
    } else {
        LOG_ERROR("Failed getLocalAddr(fd: )", socket.fd());
    }
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(
                                *reactor, connName, socket, localAddr, peerAddr);
    connections_[conn->name()] = conn;
    conn->setConnectionCallback(connectionCb_);
    conn->setMessageCallback(messageCb_);
    conn->setWriteCompleteCallback(writeCompleteCb_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    reactor->run([conn] {
        conn->connectComplete();
    });
}

void TcpServer::removeConnection(TcpConnection& conn) {
    connections_.erase(conn.name());
    Reactor& ioReactor = conn.reactor();
    ioReactor.run([&conn] {
        conn.disconnectComplete();
    });
}