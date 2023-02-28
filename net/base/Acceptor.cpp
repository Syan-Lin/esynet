#include "net/base/Acceptor.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/InetAddress.h"
#include "net/Reactor.h"

using esynet::Acceptor;

Acceptor::Acceptor(Reactor& reactor, const InetAddress& localAddr)
                    : reactor_(reactor)
                    , acceptEvent_(reactor, acceptSocket_.fd())
                    , listen_(false)
                    , port_(localAddr.port()) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bind(localAddr);
    acceptEvent_.setReadCallback(std::bind(&Acceptor::OnConnection, this));
}

Acceptor::~Acceptor() {
    acceptEvent_.disableAll();
}

void Acceptor::setConnectionCallback(ConnectionCallback cb) {
    connCb_ = cb;
}
bool Acceptor::listening() {
    return listen_;
}
void Acceptor::listen() {
    listen_ = true;
    if(!reactor_.isInLoopThread()) {
        LOG_FATAL("Call listen() in another thread: reactor({:p})", static_cast<void*>(this));
    }
    acceptSocket_.listen();
    acceptEvent_.enableReading();
}

void Acceptor::OnConnection() {
    if(!reactor_.isInLoopThread()) {
        LOG_FATAL("Call OnConnection() in another thread: reactor({:p})", static_cast<void*>(this));
    }
    InetAddress peerAddr;
    auto connSock = acceptSocket_.accept(peerAddr);
    if(connSock.has_value()) {
        if(connCb_) {
            connCb_(connSock.value(), peerAddr);
        }
    }
}