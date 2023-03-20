#include "net/Acceptor.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/NetAddress.h"
#include "net/base/Looper.h"
#include "exception/NetworkException.h"

using esynet::Acceptor;

Acceptor::Acceptor(Looper& looper, const NetAddress& localAddr):
                    looper_(looper),
                    acceptEvent_(looper, acceptSocket_.fd()),
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
    acceptEvent_.cancel();
}

void Acceptor::setAcceptCallback(AcceptCallback cb) {
    acceptCb_ = std::move(cb);
}
bool Acceptor::listening() const {
    return listen_;
}
void Acceptor::listen() {
    looper_.assert();

    listen_ = true;
    acceptSocket_.listen();
    acceptEvent_.enableRead();
}
void Acceptor::shutdown() {
    looper_.assert();

    listen_ = false;
    acceptEvent_.cancel();
}

void Acceptor::onAccept() {
    looper_.assert();

    NetAddress peerAddr;
    try {
        Socket connSocket = acceptSocket_.accept(peerAddr);
        if(acceptCb_) {
            acceptCb_(connSocket, peerAddr);
        }
    } catch(exception::NetworkException& e) {
        LOG_ERROR("{}", e.detail());
    }
}