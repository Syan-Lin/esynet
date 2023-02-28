#include "net/base/InetAddress.h"

/* Local headers */
#include "logger/Logger.h"
#include "utils/Buffer.h"
#include "net/base/Socket.h"

using esynet::InetAddress;

std::optional<InetAddress> InetAddress::resolve(utils::StringArg hostname) {
    struct hostent queryResult;
    struct hostent* result = nullptr;
    int err = 0;
    memset(&queryResult, 0, sizeof(queryResult));

    static thread_local char buf[2_KB];

    /* 线程安全 */
    int ret = gethostbyname_r(hostname.c_str(), &queryResult, buf, sizeof buf, &result, &err);
    if (ret == 0 && result != nullptr) {
        InetAddress inetAddr;
        SockAddrIn addr;
        addr.sin_addr = *reinterpret_cast<struct in_addr*>(result->h_addr);
        inetAddr.setSockAddr(*reinterpret_cast<SockAddr*>(&addr));
        return inetAddr;
    } else {
        LOG_ERROR("gethostbyname_r error(hostname: {}, err: {})", hostname.c_str(), hstrerror(err));
    }
    return std::nullopt;
}
std::optional<InetAddress> InetAddress::getLocalAddr(Socket socket) {
    InetAddress inetAddr;
    SockAddrIn addr;
    socklen_t len = sizeof addr;
    if (getsockname(socket.fd(), reinterpret_cast<SockAddr*>(&addr), &len) == 0) {
        inetAddr.setSockAddr(*reinterpret_cast<SockAddr*>(&addr));
        return inetAddr;
    } else {
        LOG_ERROR("getsockname error");
    }
    return std::nullopt;
}
std::optional<InetAddress> InetAddress::getPeerAddr(Socket socket) {
    InetAddress inetAddr;
    SockAddrIn addr;
    socklen_t len = sizeof addr;
    if (getpeername(socket.fd(), reinterpret_cast<SockAddr*>(&addr), &len) == 0) {
        inetAddr.setSockAddr(*reinterpret_cast<SockAddr*>(&addr));
        return inetAddr;
    } else {
        LOG_ERROR("getpeername error");
    }
    return std::nullopt;
}

InetAddress::InetAddress(unsigned short port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}
InetAddress::InetAddress(utils::StringArg ip, unsigned short port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

std::string InetAddress::ip() const {
    char buf[64];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}
int InetAddress::port() const {
    return be16toh(addr_.sin_port);
}

const InetAddress::SockAddr& InetAddress::getSockAddr() const {
    return *reinterpret_cast<const SockAddr*>(&addr_);
}
void InetAddress::setSockAddr(const SockAddr& addr) {
    addr_ = *reinterpret_cast<const SockAddrIn*>(&addr);
}