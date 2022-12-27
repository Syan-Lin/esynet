#pragma once

/* Standard headers */
#include <filesystem>
#include <mutex>

/* Local headers */
#include "utils/Singleton.h"
#include "utils/FileUtil.h"
#include "utils/Timestamp.h"
#include "logger/Logger.h"

/* 线程安全的同步日志后端 */
class SyncLogger : Singletonable {
public:
    SyncLogger(std::filesystem::path);
    ~SyncLogger();

    void append(const std::string&);

private:
    std::mutex mutex_;
    std::unique_ptr<FileWriter> file_;
};