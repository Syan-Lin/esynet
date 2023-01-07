#pragma once

/* Standard headers */
#include <string>

/* Linux headers */
#include <netinet/tcp.h>

/* Local headers */
#include "utils/NonCopyable.h"
#include "net/base/InetAddress.h"

namespace esynet {

class Socket : public utils::NonCopyable {
private:
    using TcpInfo = struct tcp_info;

public:
    Socket();
    Socket(int fd);
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
    const int fd_;

};

} /* namespace esynet */