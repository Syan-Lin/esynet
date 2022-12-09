#pragma once

/* System headers */
#include <filesystem>
#include <mutex>
#include <condition_variable>

/* Third-party headers */
#include "dbg.h"

/* Local headers */
#include "Singleton.h"
#include "Buffer.h"
#include "BlockingQueue.h"
#include "FileUtil.h"
#include "Timestamp.h"

/* 线程安全的异步日志后端 */
class AsyncLogger : Singletonable {
public:
    using Buffer = FixedBuffer<512 KB>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferQueue = std::queue<BufferPtr>;

    static const std::string gHeader;
    static const int gMaxFileSize;

public:
    AsyncLogger(std::filesystem::path, int = 3);
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
    std::unique_ptr<FileWriter> file_;
    BufferPtr buffer_;                  /* 当前缓冲区 */
    BufferPtr backupBuffer_;            /* 备用缓冲区 */
    BufferQueue buffers_;               /* 将要写入文件的缓存 */
};