#include <iostream>
#include <thread>

#include "net/TcpServer.h"
#include "net/TcpClient.h"

using namespace std;
using namespace esynet;

int main() {
    thread client([] {
        sleep(1);
        TcpClient client(1234, "HelloClient");
        client.setMessageCallback([](TcpConnection& conn, utils::Buffer& buffer, utils::Timestamp time) {
            LOG_INFO("Receive data from {}: {}", conn.peerAddress().ip(), buffer.retrieveAllAsString());
        });
        client.setCloseCallback([&client](TcpConnection& conn) {
            LOG_INFO("Connection from {} closed", conn.peerAddress().ip());
            client.shutdown();
        });
        client.connect();
        client.start();
    });

    TcpServer server(1234, "HelloServer");
    server.setConnectionCallback([](TcpConnection& conn) {
        LOG_INFO("New connection from {}", conn.peerAddress().ip());
        conn.send("Hello, welcome to HelloServer!");
        conn.forceClose();
    });
    server.setWriteCompleteCallback([&server](TcpConnection& conn){
        LOG_INFO("Send data to {} complete", conn.peerAddress().ip());
        // server.shutdown();
    });
    server.start();

    // client.join();

    return 0;
}