#include "net/TcpServer.h"

/* Local headers */
#include "net/base/Looper.h"

/* Standard headers */
#include <functional>

using esynet::Looper;
using esynet::TcpServer;
using esynet::ReactorThreadPoll;

TcpServer::TcpServer(NetAddress addr, utils::StringPiece name, bool useEpoll):
        looper_(useEpoll),
        port_(addr.port()),
        ip_(addr.ip()),
        name_(name.asString()),
        acceptor_(looper_, addr),
        threadPoll_(looper_) {
    connectionCb_    = TcpConnection::defaultConnectionCallback;
    writeCompleteCb_ = TcpConnection::defaultWriteCompleteCallback;
    messageCb_       = TcpConnection::defaultMessageCallback;
    closeCb_         = TcpConnection::defaultCloseCallback;
    errorCb_         = TcpConnection::defaultErrorCallback;
    acceptor_.setAcceptCallback(std::bind(&TcpServer::onConnection, this,
                                std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::start() {
    looper_.assert();

    if(started_) return;

    threadPoll_.start();
    acceptor_.listen();
    started_ = true;
    looper_.start();
}
void TcpServer::shutdown() {
    looper_.assert();

    acceptor_.shutdown();
    for(auto& iter : connections_) {
        iter.second->looper().run([&iter] {
            iter.second->forceCloseWithoutCallback();
        });
    }
    connections_.clear();
    looper_.stop();
}

const std::string&  TcpServer::ip()   const { return ip_; }
const std::string&  TcpServer::name() const { return name_; }
const int           TcpServer::port() const { return port_; }
Looper& TcpServer::looper() { return looper_; }

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

void TcpServer::onConnection(Socket socket, const NetAddress& peerAddr) {
    looper_.assert();

    Looper* looper;
    if(strategy_ == kLightest) {
        looper = threadPoll_.getLightest();
    } else {
        looper = threadPoll_.getNext();
    }
    std::string connName = name_ + "-" + peerAddr.ip() + ":"
                         + std::to_string(peerAddr.port())
                         + "-" + std::to_string(connectionCount_++);

    std::optional<NetAddress> local = NetAddress::getLocalAddr(socket);
    NetAddress localAddr;
    if(local.has_value()) {
        localAddr = local.value();
    } else {
        LOG_ERROR("Failed getLocalAddr(fd: )", socket.fd());
    }
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(
                                *looper, connName, socket, localAddr, peerAddr);
    connections_[conn->name()] = conn;
    conn->setConnectionCallback(connectionCb_);
    conn->setMessageCallback(messageCb_);
    conn->setWriteCompleteCallback(writeCompleteCb_);
    conn->setErrorCallback(errorCb_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    looper->run([conn] {
        conn->connectComplete();
    });
}

void TcpServer::removeConnection(TcpConnection& conn) {
    looper_.assert();

    connections_.erase(conn.name());
    Looper& ioReactor = conn.looper();
    closeCb_(conn);
}