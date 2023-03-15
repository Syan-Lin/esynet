#include "logger/AsyncLogger.h"

/* Local headers */
#include "logger/Logger.h"
#include "utils/Timestamp.h"

using esynet::logger::AsyncLogger;
using esynet::utils::FileWriter;
using esynet::utils::Timestamp;

AsyncLogger::AsyncLogger(std::filesystem::path path, int flushIntervalSeconds):
        file_(new FileWriter(std::filesystem::current_path()/path)),
        flushInterval_(flushIntervalSeconds),
        running_(false),
        isWriting_(false) {
    bufferForLog_   = std::make_unique<Buffer>();
    bufferForWrite_ = std::make_unique<Buffer>();
}

AsyncLogger::~AsyncLogger() {
    if(running_) {
        stop();
    }
    Logger::setLoggerDefault();
}

void AsyncLogger::start() {
    if(running_) return;
    running_ = true;
    file_->open();
    thread_ = std::thread(&AsyncLogger::writeToFile, this);
}

void AsyncLogger::stop() {
    if(!running_) return;
    running_ = false;
    cond_.notify_one();
    thread_.join();
    // 保证所有数据都提交到后端写入
    BufferQueue bufferToWrite;
    if(!bufferForWrite_->empty()) {
        extraBuffers_.push(std::move(bufferForWrite_));
    }
    if(!bufferForLog_->empty()) {
        extraBuffers_.push(std::move(bufferForLog_));
    }
    while(!extraBuffers_.empty()) {
        bufferToWrite.push(std::move(extraBuffers_.front()));
        extraBuffers_.pop();
    }
    flush(bufferToWrite);
}

void AsyncLogger::append(const std::string& data) {
    if(data.size() > bufferForLog_->capacity()) {
        std::cerr << "Data too large, ignore append" << std::endl;
        return;
    } else if(!running_) {
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    if(bufferForLog_->remain() > data.size()) {
        bufferForLog_->append(data);
    } else {
        if(!isWriting_ && bufferForWrite_->empty()) {
            std::swap(bufferForLog_, bufferForWrite_);
        } else if(extraBuffers_.size() < 64) {
            // 数据来不及写入，创建一个新的缓冲区
            extraBuffers_.push(std::move(bufferForLog_));
            bufferForLog_ = std::make_unique<Buffer>();
        } else {
            std::cerr << "Too many buffer, ignore append" << std::endl;
        }
        bufferForLog_->append(data);
        cond_.notify_one();
    }
}

void AsyncLogger::writeToFile() {
    BufferQueue bufferToWrite;
    while(running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool available = !extraBuffers_.empty() || !bufferForWrite_->empty();
            if(!available) {
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            if(bufferForWrite_->empty()) {
                std::swap(bufferForLog_, bufferForWrite_);
            }
            if(!extraBuffers_.empty()) {
                bufferToWrite = std::move(extraBuffers_);
            }
        }
        flush(bufferToWrite);
    }
}

/* 将缓冲队列写入文件，并重置备用缓冲区 */
void AsyncLogger::flush(BufferQueue& buffers) {
    auto write = [this](Buffer& buffer){
        if(file_->filesize() == 0) {
            file_->append(Logger::gHeader.c_str(), Logger::gHeader.size());
        } else if(file_->filesize() > Logger::gMaxFileSize) {
            // TODO: 配置文件化
            file_->rename(Timestamp::now().toFormattedString(false, "%Y-%m-%d %H-%M-%S") + ".log");
            file_.reset(new FileWriter(std::filesystem::current_path(), "log.log"));
            file_->append(Logger::gHeader.c_str(), Logger::gHeader.size());
        }
        file_->append(buffer.data(), buffer.length());
        buffer.reset();
    };

    if(bufferForWrite_ && !bufferForWrite_->empty()) {
        isWriting_ = true;
        write(*bufferForWrite_);
        isWriting_ = false;
    }
    while(!buffers.empty()) {
        write(*buffers.front());
        buffers.pop();
    }
}

void AsyncLogger::abort() {
    stop();
    file_.reset();
    ::abort();
}