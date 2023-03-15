#include "logger/SyncLogger.h"

/* Local headers */
#include "logger/Logger.h"
#include "utils/FileUtil.h"
#include "utils/Timestamp.h"

using esynet::logger::SyncLogger;
using esynet::utils::FileWriter;
using esynet::utils::Timestamp;

SyncLogger::SyncLogger(std::filesystem::path path) :
        file_(new FileWriter(std::filesystem::current_path()/path)) {
    file_->open();
}

SyncLogger::~SyncLogger() {
    Logger::setLoggerDefault();
}

void SyncLogger::append(const std::string& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    if(data.size() > Logger::gMaxFileSize) {
        std::cerr << "Log msg too large" << std::endl;
        return;
    }

    if(file_->filesize() == 0) {
        file_->append(Logger::gHeader.c_str(), Logger::gHeader.size());
    } else if(file_->filesize() > Logger::gMaxFileSize) {
        // TODO: 配置文件化
        file_->rename(Timestamp::now().toFormattedString(false, "%Y-%m-%d %H-%M-%S") + ".log");
        file_.reset(new FileWriter(std::filesystem::current_path(), "log.log"));
        file_->append(Logger::gHeader.c_str(), Logger::gHeader.size());
    }
    file_->append(data.c_str(), data.size());
}

void SyncLogger::abort() {
    file_.reset();
    ::abort();
}