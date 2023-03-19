#pragma once

/* Standard headers */
#include <string>
#include <memory>
#include <vector>
#include <optional>

/* Linux headers */
#include <netinet/tcp.h>

namespace esynet {

class NetAddress;

/* Socket可以任意复制，当所有副本都销毁时才会close */
class Socket {
public:
    using TcpInfo = struct tcp_info;

    static std::optional<int> getSocketError(Socket);
    static bool isSelfConnect(Socket);

public:
    Socket();
    Socket(int fd);

    int fd() const;

    void bind(const NetAddress&);
    void listen();
    // 立刻执行shutdown操作，但真正关闭会延迟到计数归零
    void close();
    /* 每次accept一个连接，适合长连接服务 */
    [[nodiscard]]
    Socket accept(NetAddress&);
    [[nodiscard]]
    Socket accept();
    /* 每次accept若干个连接，适合短连接服务 */
    [[nodiscard]]
    std::vector<Socket> accept(std::vector<NetAddress>&);
    void connect(const NetAddress&);

    std::optional<TcpInfo> getTcpInfo() const;
    std::string getTcpInfoString() const;

    void shutdownWrite();
    void shutdownRead();

    void setTcpNoDelay(bool);
    void setReuseAddr(bool);
    void setReusePort(bool);
    void setKeepAlive(bool);

    // 不建议直接使用以下接口
    size_t write(const void*, size_t);
    size_t read(void*, size_t);
    size_t readv(const struct iovec*, int);

private:
    const int fd_;
};

} /* namespace esynet */