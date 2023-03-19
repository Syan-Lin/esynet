#include "net/base/Socket.h"

/* Local headers */
#include "logger/Logger.h"
#include "net/base/NetAddress.h"
#include "utils/ErrorInfo.h"
#include "exception/SocketException.h"
#include "exception/NetworkException.h"

/* Standard headers */
#include <cstring>
#include <sstream>

/* Linux headers */
#include <sys/socket.h>
#include <sys/uio.h>

using esynet::Socket;

std::optional<int> Socket::getSocketError(Socket socket) {
    int optval;
    socklen_t optLen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(socket.fd(), SOL_SOCKET, SO_ERROR, &optval, &optLen) < 0) {
        return errno;
    }
    return std::nullopt;
}
bool Socket::isSelfConnect(Socket socket) {
    auto local = NetAddress::getLocalAddr(socket);
    auto peer = NetAddress::getLocalAddr(socket);
    if(local.has_value() && peer.has_value()) {
        struct sockaddr_in6&
        localAddr = (sockaddr_in6&)local.value().getSockAddr();
        struct sockaddr_in6&
        peerAddr = (sockaddr_in6&)peer.value().getSockAddr();
        const struct sockaddr_in* laddr4 = (struct sockaddr_in*)(&localAddr);
        const struct sockaddr_in* raddr4 = (struct sockaddr_in*)(&peerAddr);
        return (laddr4->sin_port == raddr4->sin_port)
                    && (laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr);
    }
    return false;
}

Socket::Socket() : fd_(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) {
    if(fd_ == -1) {
        throw exception::SocketException("Create socket failed", errno);
    }
    LOG_DEBUG("create socket {}", fd_);
}
Socket::Socket(int fd) : fd_(fd) {}

int Socket::fd() const {
    return fd_;
}

void Socket::bind(const NetAddress& addr) {
    auto sa = addr.getSockAddr();
    if(::bind(fd_, &sa, sizeof sa) == -1) {
        throw exception::NetworkException("Bind failed(fd: " + std::to_string(fd_) + ")", errno);
    }
}
void Socket::listen() {
    if(::listen(fd_, SOMAXCONN) == -1) {
        throw exception::NetworkException("Listen failed(fd: " + std::to_string(fd_) + ")", errno);
    }
}
void Socket::close() {
    if(::close(fd_) == -1) {
        LOG_ERROR("close error(fd: {}, errno: {})", fd_, errnoStr(errno));
    }
    LOG_DEBUG("close socket {}", fd_);
}
Socket Socket::accept() {
    NetAddress peerAddr;
    return accept(peerAddr);
}
Socket Socket::accept(NetAddress& peerAddr) {
    NetAddress::SockAddr addr;
    socklen_t len;
    int connFd = ::accept4(fd_, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connFd == -1) {
        throw exception::NetworkException("Accept failed(fd: " + std::to_string(fd_) + ")", errno);
    }
    peerAddr.setSockAddr(addr);
    return connFd;
}
std::vector<Socket> Socket::accept(std::vector<NetAddress>& peerAddrs) {
    std::vector<Socket> fds;
    while(true) {
        NetAddress::SockAddr addr;
        socklen_t len;
        int connFd = ::accept4(fd_, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(connFd == -1) {
            if(!(errno == EINTR || errno == EMFILE || errno == ECONNABORTED)
                    && (errno == ENFILE || errno == ENOMEM)) {
                throw exception::NetworkException("Accept failed(fd: " + std::to_string(fd_) + ")", errno);
            }
            break;
        } else {
            fds.push_back(connFd);
            NetAddress peerAddr;
            peerAddr.setSockAddr(addr);
            peerAddrs.push_back(peerAddr);
        }
    }
    return fds;
}
void Socket::connect(const NetAddress& peerAddr) {
    auto& addr = peerAddr.getSockAddr();
    if(::connect(fd_, &addr, sizeof addr) == -1) {
        throw exception::NetworkException("Connect failed(fd: " + std::to_string(fd_) + ")", errno);
    }
}

std::optional<Socket::TcpInfo> Socket::getTcpInfo() const {
    TcpInfo info;
    socklen_t len = sizeof info;
    memset(&info, 0, len);
    if(getsockopt(fd_, SOL_TCP, TCP_INFO, &info, &len) == -1) {
        return std::nullopt;
    }
    return info;
}

std::string Socket::getTcpInfoString() const {
    auto tcpInfo = getTcpInfo();
    std::stringstream ss;
    if(tcpInfo.has_value()) {
        auto& val = tcpInfo.value();
        ss << "retransmits = " << val.tcpi_retransmits
        << ", rto = " << val.tcpi_rto << ", ato = " << val.tcpi_ato
        << ", snd_mss = " << val.tcpi_snd_mss << ", rcv_mss = " << val.tcpi_rcv_mss
        << ", lost = " << val.tcpi_lost << ", retrans = " << val.tcpi_retrans
        << ", rtt = " << val.tcpi_rtt << ", rttvar = " << val.tcpi_rttvar
        << ", ssthresh = " << val.tcpi_snd_ssthresh << ", cwnd = " << val.tcpi_snd_cwnd
        << ", total_retrans = " << val.tcpi_total_retrans;
    }
    return ss.str();
}

void Socket::shutdownWrite() {
    if(shutdown(fd_, SHUT_WR) == -1) {
        LOG_ERROR("shutdownWrite failed(fd: {}, errno: )", fd_, errnoStr(errno));
    }
}
void Socket::shutdownRead() {
    if(shutdown(fd_, SHUT_RD) == -1) {
        LOG_ERROR("shutdownRead failed(fd: {}, errno: )", fd_, errnoStr(errno));
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval) == -1) {
        LOG_ERROR("setTcpNoDelay failed(fd: {}, errno: )", fd_, errnoStr(errno));
    }
}
void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(fd_, IPPROTO_TCP, SO_REUSEADDR, &optval, sizeof optval) == -1) {
        LOG_ERROR("setReuseAddr failed(fd: {}, errno: )", fd_, errnoStr(errno));
    }
}
void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(fd_, IPPROTO_TCP, SO_REUSEPORT, &optval, sizeof optval) == -1) {
        LOG_ERROR("setReusePort failed(fd: {}, errno: )", fd_, errnoStr(errno));
    }
}
void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    if(setsockopt(fd_, IPPROTO_TCP, SO_KEEPALIVE, &optval, sizeof optval) == -1) {
        LOG_ERROR("setKeepAlive failed(fd: {}, errno: )", fd_, errnoStr(errno));
    }
}

size_t Socket::write(const void* data, size_t len) {
    size_t bytes = ::write(fd_, data, len);
    if(bytes < 0) {
        throw exception::SocketException("Write error(fd: " + std::to_string(fd_) + ")", errno);
    }
    return bytes;
}
size_t Socket::read(void* buf, size_t len) {
    size_t bytes = ::read(fd_, buf, len);
    if(bytes < 0) {
        throw exception::SocketException("Read error(fd: " + std::to_string(fd_) + ")", errno);
    }
    return bytes;
}
size_t Socket::readv(const struct iovec* iov, int iovCount) {
    size_t bytes = ::readv(fd_, iov, iovCount);
    if(bytes < 0) {
        throw exception::SocketException("Readv error(fd: " + std::to_string(fd_) + ")", errno);
    }
    return bytes;
}