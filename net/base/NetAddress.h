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
class NetAddress {
public:
    /* 根据域名解析IP地址 */
    static std::optional<NetAddress> resolve(utils::StringArg);
    /* 需要该 Socket 已 bind 或已 connect */
    static std::optional<NetAddress> getLocalAddr(Socket);
    static std::optional<NetAddress> getPeerAddr(Socket);

    using SockAddr   = struct sockaddr;
    using SockAddrIn = struct sockaddr_in;

public:
    NetAddress(unsigned short port = 0);  /* 用于服务端 */
    NetAddress(utils::StringArg ip, unsigned short port);

    std::string ip() const;
    int port() const;

    const SockAddr& getSockAddr() const;
    void setSockAddr(const SockAddr&);

private:
    SockAddrIn addr_;
};

} /* namespace esynet */