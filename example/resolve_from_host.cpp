#include "net/base/NetAddress.h"
#include "net/base/Socket.h"
#include <iostream>

using namespace std;
using namespace esynet;

int main() {
    /* 根据域名获取 IP */
    string host = "www.bilibili.com";
    auto result = NetAddress::resolve(host);
    if (result.has_value()) {
        cout << "resolve " << host << " success: " << result.value().ip() << endl;
    } else {
        cout << "resolve " << host << " failed" << endl;
    }

    return 0;
}