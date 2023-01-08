#pragma once

/* Standard headers */
#include <string>
#include <memory>

/* Linux headers */
#include <netinet/tcp.h>

/* Local headers */
#include "utils/NonCopyable.h"
#include "net/base/InetAddress.h"

namespace esynet {

/* Socket可以任意复制，当所有副本都销毁时才会close */
class Socket {
public:
    using TcpInfo = struct tcp_info;

public:
    Socket();
    Socket(int fd);
    Socket(const Socket&);
    Socket& operator=(const Socket&);
    Socket(Socket&&);
    Socket& operator=(Socket&&);
    ~Socket();

    int fd() const;

    void bind(const InetAddress&);
    void listen();
    int accept(InetAddress&);
    int accept();
    void connect(const InetAddress&);

    TcpInfo getTcpInfo() const;
    std::string getTcpInfoString() const;

    void shutdownWrite();
    void shutdownRead();

    void setTcpNoDelay(bool);
    void setReuseAddr(bool);
    void setReusePort(bool);
    void setKeepAlive(bool);

private:
    std::shared_ptr<const int> fd_;
};

} /* namespace esynet */