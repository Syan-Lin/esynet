#pragma once

/* Standard headers */
#include <filesystem>
#include <mutex>

/* Local headers */
#include "utils/Singleton.h"
#include "utils/FileUtil.h"

namespace esynet::logger {

class SyncLogger : utils::Singletonable {
public:
    SyncLogger(std::filesystem::path);
    ~SyncLogger();

    void append(const std::string&);

    void abort();

private:
    std::mutex mutex_;
    std::unique_ptr<utils::FileWriter> file_;
};

} /* namespace esynet::logger */