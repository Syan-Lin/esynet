#include "net/TcpServer.h"

/* Local headers */
#include "net/base/Reactor.h"

/* Standard headers */
#include <functional>

using esynet::Reactor;
using esynet::TcpServer;
using esynet::ReactorThreadPoll;

TcpServer::TcpServer(InetAddress addr, utils::StringPiece name, bool useEpoll):
        reactor_(useEpoll),
        port_(addr.port()),
        ip_(addr.ip()),
        name_(name.asString()),
        acceptor_(reactor_, addr),
        threadPoll_(reactor_),
        strategy_(kRoundRobin),
        started_(false),
        connectionCount_(1) {
    connectionCb_    = TcpConnection::defaultConnectionCallback;
    writeCompleteCb_ = TcpConnection::defaultWriteCompleteCallback;
    messageCb_       = TcpConnection::defaultMessageCallback;
    closeCb_         = TcpConnection::defaultCloseCallback;
    errorCb_         = TcpConnection::defaultErrorCallback;
    acceptor_.setAcceptCallback(std::bind(&TcpServer::onConnection, this,
                                std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    shutdown();
}

void TcpServer::start() {
    reactor_.assert();
    if(!started_) {
        threadPoll_.start();
        acceptor_.listen();
    }
    started_ = true;
    reactor_.start();
}
void TcpServer::shutdown() {
    reactor_.assert();
    for(auto& iter : connections_) {
        iter.second->reactor().run([&iter] {
            iter.second->disconnectComplete();
        });
    }
    connections_.clear();
    reactor_.stop();
}

const std::string&  TcpServer::ip()   const { return ip_; }
const std::string&  TcpServer::name() const { return name_; }
const int           TcpServer::port() const { return port_; }
Reactor& TcpServer::reactor() { return reactor_; }

void TcpServer::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCb_ = std::move(cb);
}
void TcpServer::setMessageCallback(const MessageCallback& cb) {
    messageCb_ = std::move(cb);
}
void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCb_ = std::move(cb);
}
void TcpServer::setCloseCallback(const CloseCallback& cb) {
    closeCb_ = std::move(cb);
}
void TcpServer::setErrorCallback(const ErrorCallback& cb) {
    errorCb_ = std::move(cb);
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback& cb) {
    threadPoll_.setInitCallback(std::move(cb));
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
                         + std::to_string(peerAddr.port())
                         + "-" + std::to_string(connectionCount_++);

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
    conn->setErrorCallback(errorCb_);
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
    closeCb_(conn);
}