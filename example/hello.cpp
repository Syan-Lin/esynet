#include <iostream>
#include <thread>

#include "net/TcpServer.h"
#include "net/TcpClient.h"

using namespace std;
using namespace esynet;

int main() {
    string msg;
    thread client([&msg] {
        sleep(1);
        TcpClient client({"0.0.0.0", 1234}, "HelloClient");
        client.setMessageCallback([&msg](TcpConnection& conn, utils::Buffer& buffer, utils::Timestamp time) {
            msg = buffer.retrieveAllAsString();
            LOG_INFO("Receive data from {}: {}", conn.peerAddress().ip(), msg);
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
    server.setCloseCallback([&server](TcpConnection& conn){
        LOG_INFO("Connection close {}", conn.peerAddress().ip());
        server.shutdown();
    });
    server.start();

    client.join();

    cout << "==========Receive Message==========" << endl;
    cout << msg << endl;
    cout << "===================================" << endl;

    return 0;
}