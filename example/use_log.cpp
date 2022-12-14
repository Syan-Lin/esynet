#include "logger/Logger.h"
#include "logger/SyncLogger.h"
#include "logger/AsyncLogger.h"

using namespace std;
using namespace esynet;
using namespace esynet::logger;

void synclog(std::string& path) {
    SyncLogger sl(path);
    Logger::setLogger(sl);
    LOG_INFO("Hello Wolrd!");
}

void asynclog(std::string& path) {
    /* 第二个参数代表每多少秒将数据写入文件，如果数据量不够填满一个缓冲区 */
    AsyncLogger al("log/log.log", 1);
    Logger::setLogger(al);
    al.start(); /* 必须 */
    LOG_INFO("Hello Wolrd!");
    sleep(1);   /* 销毁过快可能导致数据来不及提交后端就结束 */
    al.stop();  /* 非必须，在析构时会自动 stop */
}

void customlog() {
    auto logToCout = [](const std::string& msg) {
        std::cout << msg;
    };
    Logger::setLogger(logToCout);
    LOG_INFO("Hello Wolrd!");
}

class CustomLogger {
public:
    void append(const string& msg) {
        std::cout << msg;
    }
};

void customObj() {
    CustomLogger cst;
    Logger::setLogger(std::bind(&CustomLogger::append, &cst, std::placeholders::_1));
    LOG_INFO("Hello Wolrd!");
    /* 注意对象被销毁 Logger 还在使用的问题，请手动注销 Logger 的绑定 */
    Logger::setLoggerDefault();
}

int main() {
    /* 目录基于执行文件的相对路径 */
    std::string filepath = "log/log.log";

    /* 注意：对应的输出行为仅在后端对象存在时有效，如日志后端对象销毁
     * 则日志系统会将日志写入到默认输出流(cout)
     */
    synclog(filepath);  /* 线程安全的同步日志 */
    LOG_INFO("should print to console");
    asynclog(filepath); /* 线程安全的异步日志 */
    LOG_INFO("should print to console");
    customlog();        /* 自定义日志函数 */
    LOG_INFO("should print to console");
    customObj();        /* 自定义日志后端 */
    LOG_INFO("should print to console");

    /* TODO: 一些可选设置:
     * 1. 设置日志级别
     * 2. 设置日志格式（文件头/消息格式）
     * 3. 设置日志文件最大大小
     * 4. 设置日志文件命名
     */

    return 0;
}