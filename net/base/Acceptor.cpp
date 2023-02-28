#include "net/base/Acceptor.h"

/* Local headers */
#include "logger/Logger.h"

using esynet::Acceptor;

Acceptor::Acceptor(EventLoop& loop, const InetAddress& localAddr)
                    : loop_(loop), acceptEvent_(loop, acceptSocket_.fd()),
                      listen_(false), port_(localAddr.port()) {
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
    if(!loop_.isInLoopThread()) {
        LOG_FATAL("Listen not in loop({:p}) thread", static_cast<void*>(this));
    }
    acceptSocket_.listen();
    acceptEvent_.enableReading();
}

void Acceptor::OnConnection() {
    if(!loop_.isInLoopThread()) {
        LOG_FATAL("Call OnConnection() not in loop({:p}) thread", static_cast<void*>(this));
    }
    InetAddress peerAddr;
    auto connSock = acceptSocket_.accept(peerAddr);
    if(connSock.has_value()) {
        if(connCb_) {
            connCb_(connSock.value(), peerAddr);
        }
    }
}