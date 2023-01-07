#pragma once

/* Linux headers */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Local headers */
#include "utils/StringPiece.h"

namespace esynet {

/* IPv4 地址 TODO: IPv6 */
class InetAddress {
public:
    /* 根据域名解析IP地址，失败设端口为-1 */
    static InetAddress resolve(utils::StringArg);

    using SockAddr = struct sockaddr;
    using SockAddrIn = struct sockaddr_in;

public:
    InetAddress(int port);  /* 用于服务端 */
    InetAddress(utils::StringArg ip, int port);

    std::string ip() const;
    int port() const;

    const SockAddr& getSockAddr() const;
    void setSockAddr(const SockAddr&);

private:
    SockAddrIn addr_;
};

} /* namespace esynet */