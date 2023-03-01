#include "net/TcpServer.h"

/* Local headers */
#include "net/Reactor.h"
#include "net/Event.h"

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
    acceptor_.setConnectionCallback(std::bind(
        &TcpServer::OnConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for(auto& iter : connections_) {
        iter.second->reactor().run([&iter] {
            iter.second->connectDestroyed();
        });
    }
    connections_.clear();
}

void TcpServer::start() {
    if(!reactor_.isInLoopThread()) {
        LOG_FATAL("Try start TcpServer in another thread(reactor: {:p})",
                    static_cast<void*>(&reactor_));
    }
    if(!started_) {
        threadPoll_.start();
        reactor_.run([this] {
            this->acceptor_.listen();
        });
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

void TcpServer::OnConnection(Socket socket, const InetAddress& peerAddr) {
    if(!reactor_.isInLoopThread()) {
        LOG_FATAL("TcpServer OnConnection in another thread(reactor: {:p})",
                    static_cast<void*>(&reactor_));
    }
    Reactor* reactor;
    if(strategy_ == kLightest) {
        reactor = threadPoll_.getLightest();
    } else {
        reactor = threadPoll_.getNext();
    }
    std::string connName = name_ + "-" + peerAddr.ip() + ":"
                            + std::to_string(peerAddr.port()) + "-" + std::to_string(nextConnId_++);

    auto local = InetAddress::getLocalAddr(socket);
    InetAddress localAddr;
    if(local.has_value()) {
        localAddr = local.value();
    }
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(
                                *reactor, connName, socket, localAddr, peerAddr);
    connections_[conn->name()] = conn;
    conn->setConnectionCallback(connectionCb_);
    conn->setMessageCallback(messageCb_);
    conn->setWriteCompleteCallback(writeCompleteCb_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    reactor->run([conn] {
        conn->connectEstablished();
    });
}

void TcpServer::removeConnection(TcpConnection& conn) {
    connections_.erase(conn.name());
    Reactor& ioReactor = conn.reactor();
    ioReactor.run([&conn] {
        conn.connectDestroyed();
    });
}