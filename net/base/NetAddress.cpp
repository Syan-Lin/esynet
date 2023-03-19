#include "net/base/NetAddress.h"

/* Local headers */
#include "logger/Logger.h"
#include "utils/Buffer.h"
#include "net/base/Socket.h"
#include "utils/ErrorInfo.h"

using esynet::NetAddress;

std::optional<NetAddress> NetAddress::resolve(utils::StringArg hostname) {
    struct hostent queryResult;
    struct hostent* result = nullptr;
    int err = 0;
    memset(&queryResult, 0, sizeof(queryResult));

    static thread_local char buf[2_KB];

    /* 线程安全 */
    int ret = gethostbyname_r(hostname.c_str(), &queryResult, buf, sizeof buf, &result, &err);
    if (ret == 0 && result != nullptr) {
        NetAddress netAddr;
        SockAddrIn addr;
        addr.sin_addr = *reinterpret_cast<struct in_addr*>(result->h_addr);
        netAddr.setSockAddr(*reinterpret_cast<SockAddr*>(&addr));
        return netAddr;
    } else {
        LOG_ERROR("gethostbyname_r error(hostname: {}, err: {})", hostname.c_str(), errnoStr(err));
    }
    return std::nullopt;
}
std::optional<NetAddress> NetAddress::getLocalAddr(Socket socket) {
    NetAddress netAddr;
    SockAddrIn addr;
    socklen_t len = sizeof addr;
    if (getsockname(socket.fd(), reinterpret_cast<SockAddr*>(&addr), &len) == 0) {
        netAddr.setSockAddr(*reinterpret_cast<SockAddr*>(&addr));
        return netAddr;
    } else {
        LOG_DEBUG("getsockname error(fd: {}, err: {})", socket.fd(), errnoStr(errno));
    }
    return std::nullopt;
}
std::optional<NetAddress> NetAddress::getPeerAddr(Socket socket) {
    NetAddress netAddr;
    SockAddrIn addr;
    socklen_t len = sizeof addr;
    if (getpeername(socket.fd(), reinterpret_cast<SockAddr*>(&addr), &len) == 0) {
        netAddr.setSockAddr(*reinterpret_cast<SockAddr*>(&addr));
        return netAddr;
    } else {
        LOG_DEBUG("getpeername error(fd: {}, err: {})", socket.fd(), errnoStr(errno));
    }
    return std::nullopt;
}

NetAddress::NetAddress(unsigned short port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}
NetAddress::NetAddress(utils::StringArg ip, unsigned short port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

std::string NetAddress::ip() const {
    char buf[64];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}
int NetAddress::port() const {
    return be16toh(addr_.sin_port);
}

const NetAddress::SockAddr& NetAddress::getSockAddr() const {
    return *reinterpret_cast<const SockAddr*>(&addr_);
}
void NetAddress::setSockAddr(const SockAddr& addr) {
    addr_ = *reinterpret_cast<const SockAddrIn*>(&addr);
}