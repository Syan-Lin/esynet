#include "net/base/Socket.h"
#include "logger/Logger.h"
#include "net/base/InetAddress.h"

/* Standard headers */
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cstring>

/* Linux headers */
#include <sys/socket.h>

using esynet::Socket;

Socket::Socket() : fd_(std::make_shared<const int>(
                        socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))) {
    if(*fd_ == -1) {
        LOG_ERROR("Create socket failed");
    }
}
Socket::Socket(int fd) : fd_(std::make_shared<const int>(fd)) {}
Socket::Socket(const Socket& sock) : fd_(sock.fd_) {}
Socket::Socket(Socket&& sock) : fd_(std::move(sock.fd_)) {}
Socket& Socket::operator=(const Socket& sock) {
    fd_ = sock.fd_;
    return *this;
}
Socket& Socket::operator=(Socket&& sock) {
    fd_ = std::move(sock.fd_);
    return *this;
}
Socket::~Socket() {
    if(fd_.unique()) {
        if(close(*fd_) == -1) {
            LOG_ERROR("close error(fd: {}, errno: {})", *fd_, strerror(errno));
        }
    }
}

int Socket::fd() const {
    return *fd_;
}

void Socket::bind(const InetAddress& addr) {
    auto sa = addr.getSockAddr();
    if(::bind(*fd_, &sa, sizeof sa) == -1) {
        LOG_FATAL("bind failed(fd: {}, errno: {})", *fd_, strerror(errno));
    }
}
void Socket::listen() {
    if(::listen(*fd_, SOMAXCONN) == -1) {
        LOG_FATAL("listen failed(fd: {}, errno: {})", *fd_, strerror(errno));
    }
}
int Socket::accept() {
    InetAddress peerAddr;
    return accept(peerAddr);
}
int Socket::accept(InetAddress& peerAddr) {
    InetAddress::SockAddr addr;
    socklen_t len;
    int connFd = ::accept4(*fd_, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connFd == -1) {
        if(errno == EINTR || errno == EMFILE || errno == ECONNABORTED) {
            LOG_INFO("accept failed(fd: {}, errno: {})", *fd_, strerror(errno));
        } else if(errno == ENFILE || errno == ENOMEM) {
            LOG_FATAL("accept failed(fd: {}, errno: {})", *fd_, strerror(errno));
        } else {
            LOG_ERROR("accept failed(fd: {}, errno: {})", *fd_, strerror(errno));
        }
    }
    peerAddr.setSockAddr(addr);
    return connFd;
}
void Socket::connect(const InetAddress& peerAddr) {
    auto& addr = peerAddr.getSockAddr();
    if(::connect(*fd_, &addr, sizeof addr) == -1) {
        LOG_ERROR("connect failed(fd: {}, errno: {})", *fd_, strerror(errno));
    }
}

Socket::TcpInfo Socket::getTcpInfo() const {
    TcpInfo info;
    socklen_t len = sizeof info;
    memset(&info, 0, len);
    if(getsockopt(*fd_, SOL_TCP, TCP_INFO, &info, &len) == -1) {
        LOG_ERROR("getsockopt failed(fd: {}, errno: {})", *fd_, strerror(errno));
    }
    return info;
}
std::string Socket::getTcpInfoString() const {
    auto tcpInfo = getTcpInfo();
    std::stringstream ss;
    ss << "retransmits = " << tcpInfo.tcpi_retransmits
       << ", rto = " << tcpInfo.tcpi_rto << ", ato = " << tcpInfo.tcpi_ato
       << ", snd_mss = " << tcpInfo.tcpi_snd_mss << ", rcv_mss = " << tcpInfo.tcpi_rcv_mss
       << ", lost = " << tcpInfo.tcpi_lost << ", retrans = " << tcpInfo.tcpi_retrans
       << ", rtt = " << tcpInfo.tcpi_rtt << ", rttvar = " << tcpInfo.tcpi_rttvar
       << ", ssthresh = " << tcpInfo.tcpi_snd_ssthresh << ", cwnd = " << tcpInfo.tcpi_snd_cwnd
       << ", total_retrans = " << tcpInfo.tcpi_total_retrans;
    return ss.str();
}

void Socket::shutdownWrite() {
    if(shutdown(*fd_, SHUT_WR) == -1) {
        LOG_ERROR("shutdownWrite failed(fd: {}, errno: )", *fd_, strerror(errno));
    }
}
void Socket::shutdownRead() {
    if(shutdown(*fd_, SHUT_RD) == -1) {
        LOG_ERROR("shutdownRead failed(fd: {}, errno: )", *fd_, strerror(errno));
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(*fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval) == -1) {
        LOG_ERROR("setTcpNoDelay failed(fd: {}, errno: )", *fd_, strerror(errno));
    }
}
void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(*fd_, IPPROTO_TCP, SO_REUSEADDR, &optval, sizeof optval) == -1) {
        LOG_ERROR("setReuseAddr failed(fd: {}, errno: )", *fd_, strerror(errno));
    }
}
void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(*fd_, IPPROTO_TCP, SO_REUSEPORT, &optval, sizeof optval) == -1) {
        LOG_ERROR("setReusePort failed(fd: {}, errno: )", *fd_, strerror(errno));
    }
}
void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(*fd_, IPPROTO_TCP, SO_KEEPALIVE, &optval, sizeof optval) == -1) {
        LOG_ERROR("setKeepAlive failed(fd: {}, errno: )", *fd_, strerror(errno));
    }
}