#pragma once

/* Standard headers */
#include <optional>

/* Linux headers */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Local headers */
#include "utils/StringPiece.h"

namespace esynet {

class Socket;

/* IPv4 地址 */
class InetAddress {
public:
    /* 根据域名解析IP地址 */
    static std::optional<InetAddress> resolve(utils::StringArg);
    /* 需要该 Socket 已 bind 或已 connect */
    static std::optional<InetAddress> getLocalAddr(Socket);
    static std::optional<InetAddress> getPeerAddr(Socket);

    using SockAddr   = struct sockaddr;
    using SockAddrIn = struct sockaddr_in;

public:
    InetAddress(unsigned short port = 0);  /* 用于服务端 */
    InetAddress(utils::StringArg ip, unsigned short port);

    std::string ip() const;
    int port() const;

    const SockAddr& getSockAddr() const;
    void setSockAddr(const SockAddr&);

private:
    SockAddrIn addr_;
};

} /* namespace esynet */