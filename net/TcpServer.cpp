#include "net/TcpServer.h"
#include "net/Event.h"
#include <functional>

/* Local headers */

/* Linux headers */

/* Standard headers */

using esynet::EventLoop;
using esynet::TcpServer;
using esynet::EventLoopThreadPoll;

TcpServer::TcpServer(EventLoop& loop, InetAddress addr, utils::StringPiece name):
        loop_(loop),
        port_(addr.port()),
        ip_(addr.ip()),
        name_(name.asString()),
        acceptor_(loop, addr),
        threadPoll_(loop),
        started_(false) {
    connectionCb_ = TcpConnection::defaultConnectionCallback;
    messageCb_ = TcpConnection::defaultMessageCallback;
    acceptor_.setConnectionCallback(std::bind(
        &TcpServer::OnConnection, this,std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for(auto& iter : connections_) {
        iter.second->getLoop().run([iter] {
            iter.second->connectDestroyed();
        });
    }
    connections_.clear();
}

void TcpServer::start() {
    if(!loop_.isInLoopThread()) {
        LOG_FATAL("Try start TcpServer in another thread(loop: {:p})",
                    static_cast<void*>(&loop_));
    }
    if(!started_) {
        threadPoll_.start();
        loop_.run([this] {
            this->acceptor_.listen();
        });
    }
    started_ = true;
}

const std::string&  TcpServer::ip() const   { return ip_; }
const std::string&  TcpServer::name() const { return name_; }
int                 TcpServer::port() const { return port_; }
EventLoop&          TcpServer::getLoop()    { return loop_; }

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

EventLoopThreadPoll& TcpServer::threadPoll() {
    return threadPoll_;
}
void TcpServer::setThreadPollStrategy(Strategy strategy) {
    strategy_ = strategy;
}

void TcpServer::OnConnection(Socket sock, const InetAddress& peerAddr) {
    if(!loop_.isInLoopThread()) {
        LOG_FATAL("TcpServer OnConnection in another thread(loop: {:p})",
                    static_cast<void*>(&loop_));
    }
    EventLoop* loop;
    if(strategy_ == kLightest) {
        loop = threadPoll_.getLightest();
    } else {
        loop = threadPoll_.getNext();
    }
    LOG_INFO("TcpServer::OnConnection new connection from {}", peerAddr.ip());

    auto localAddr = InetAddress::getLocalAddr(sock);
    TcpConnectionPtr conn = std::make_shared<TcpConnection>( // TODO: 修改
                                *loop, std::move(sock), localAddr, peerAddr);
    connections_[conn->name()] = conn;
    conn->setConnectionCallback(connectionCb_);
    conn->setMessageCallback(messageCb_);
    conn->setWriteCompleteCallback(writeCompleteCb_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    loop->run([conn] {
        conn->connectEstablished();
    });
}

void TcpServer::removeConnection(const TcpConnection& conn) {
    LOG_INFO("TcpServer::removeConnection {}", conn.peerAddress().ip());
    connections_.erase(conn.name());
    EventLoop& ioLoop = conn.getLoop();
    ioLoop.run(std::bind(&TcpConnection::connectDestroyed, conn));
}