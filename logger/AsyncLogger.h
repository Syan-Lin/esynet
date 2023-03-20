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

class AsyncLogger : utils::Singletonable {
public:
    using Buffer = utils::LogBuffer<4_MB>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferQueue = std::queue<BufferPtr>;

public:
    AsyncLogger(std::filesystem::path, int flushIntervalSeconds = 3);
    ~AsyncLogger();

    void start();
    void stop();
    void append(const std::string&);

    void abort();

private:
    void flush(BufferQueue&);
    void writeToFile();

private:
    const int flushInterval_;

    std::mutex mutex_;
    std::thread thread_;
    std::atomic<bool> running_;
    std::atomic<bool> isWriting_;
    std::condition_variable cond_;

    BufferPtr bufferForLog_;            /* log 写入缓冲区 */
    BufferPtr bufferForWrite_;          /* 写入文件缓冲区 */
    BufferQueue extraBuffers_;          /* 面对突发大数据的缓冲区 */
    std::unique_ptr<utils::FileWriter> file_;
};

} /* namespace esynet::logger */