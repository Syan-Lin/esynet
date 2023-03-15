#pragma once

/* Standard headers */
#include <string>
#include <memory>
#include <vector>
#include <optional>

/* Linux headers */
#include <netinet/tcp.h>

namespace esynet {

class InetAddress;

/* Socket可以任意复制，当所有副本都销毁时才会close */
class Socket {
public:
    using TcpInfo = struct tcp_info;

    static std::optional<int> getSocketError(Socket);
    static bool isSelfConnect(Socket);

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
    // 立刻执行shutdown操作，但真正关闭会延迟到计数归零
    void close();
    /* 每次accept一个连接，适合长连接服务 */
    [[nodiscard]]
    std::optional<int> accept(InetAddress&);
    [[nodiscard]]
    std::optional<int> accept();
    /* 每次accept若干个连接，适合短连接服务 */
    [[nodiscard]]
    std::vector<int> accept(std::vector<InetAddress>&);
    std::optional<int> connect(const InetAddress&);

    std::optional<TcpInfo> getTcpInfo() const;
    std::string getTcpInfoString() const;

    void shutdownWrite();
    void shutdownRead();

    void setTcpNoDelay(bool);
    void setReuseAddr(bool);
    void setReusePort(bool);
    void setKeepAlive(bool);

    // 不建议直接使用以下接口，用于内部使用
    size_t write(const void*, size_t);
    size_t read(void*, size_t);
    size_t readv(const struct iovec*, int);

private:
    std::shared_ptr<const int> fd_;
};

} /* namespace esynet */