#include "net/base/Acceptor.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/InetAddress.h"
#include "net/Reactor.h"
#include "exception/SocketException.h"

using esynet::Acceptor;

Acceptor::Acceptor(Reactor& reactor, const InetAddress& localAddr):
                    reactor_(reactor),
                    acceptEvent_(reactor, acceptSocket_.fd()),
                    listen_(false),
                    port_(localAddr.port()) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptEvent_.setReadCallback(std::bind(&Acceptor::onConnection, this));
    try {
        acceptSocket_.bind(localAddr);
    } catch(exception::SocketException& e) {
        LOG_FATAL("{}", e.detail());
    }
}

Acceptor::~Acceptor() {
    acceptEvent_.disableAll();
}

void Acceptor::setConnectionCallback(ConnectionCallback cb) {
    connCb_ = cb;
}
bool Acceptor::listening() const {
    return listen_;
}
void Acceptor::listen() {
    reactor_.assert();

    listen_ = true;
    acceptSocket_.listen();
    acceptEvent_.enableRead();
}

void Acceptor::onConnection() {
    reactor_.assert();

    InetAddress peerAddr;
    try {
        Socket connSocket = acceptSocket_.accept(peerAddr);
        if(connCb_) {
            connCb_(connSocket, peerAddr);
        }
    } catch(exception::SocketException& e) {
        LOG_ERROR("{}", e.detail());
    }
}