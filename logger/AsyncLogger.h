#pragma once

/* Standard headers */
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <queue>

/* Local headers */
#include "utils/Singleton.h"
#include "utils/FileUtil.h"

namespace esynet::logger {

/* 线程安全的异步日志后端 */
class AsyncLogger : utils::Singletonable {
public:
    using Buffer = utils::LogBuffer<4_MB>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferQueue = std::queue<BufferPtr>;

public:
    AsyncLogger(std::filesystem::path, int flushInterval = 3);
    ~AsyncLogger();

    void start();
    void stop();
    void append(const std::string&);

private:
    void flush(BufferQueue&);
    void writeToFile();

private:
    const int flushInterval_;
    std::mutex mutex_;
    std::thread thread_;
    std::atomic<bool> running_;
    std::condition_variable cond_;
    std::unique_ptr<utils::FileWriter> file_;
    BufferPtr buffer_;                  /* 当前缓冲区 */
    BufferPtr backupBuffer_;            /* 备用缓冲区 */
    BufferQueue buffers_;               /* 将要写入文件的缓存 */
};

} /* namespace esynet::logger */