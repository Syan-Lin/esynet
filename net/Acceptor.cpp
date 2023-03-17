#include "net/Acceptor.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/InetAddress.h"
#include "net/base/Reactor.h"
#include "exception/NetworkException.h"

using esynet::Acceptor;

Acceptor::Acceptor(Reactor& reactor, const InetAddress& localAddr):
                    reactor_(reactor),
                    acceptEvent_(reactor, acceptSocket_.fd()),
                    listen_(false),
                    port_(localAddr.port()) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptEvent_.setReadCallback(std::bind(&Acceptor::onAccept, this));
    try {
        acceptSocket_.bind(localAddr);
    } catch(exception::NetworkException& e) {
        LOG_FATAL("{}", e.detail());
    }
}

Acceptor::~Acceptor() {
    acceptEvent_.disableAll();
}

void Acceptor::setAcceptCallback(AcceptCallback cb) {
    acceptCb_ = std::move(cb);
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

void Acceptor::onAccept() {
    reactor_.assert();

    InetAddress peerAddr;
    try {
        Socket connSocket = acceptSocket_.accept(peerAddr);
        if(acceptCb_) {
            acceptCb_(connSocket, peerAddr);
        }
    } catch(exception::NetworkException& e) {
        LOG_ERROR("{}", e.detail());
    }
}