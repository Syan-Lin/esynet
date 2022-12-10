#include "AsyncLogger.h"

AsyncLogger::AsyncLogger(std::filesystem::path path, int flushInterval) :
        file_(new FileWriter(std::filesystem::current_path()/path)), flushInterval_(flushInterval)  {
    buffer_ = std::make_unique<Buffer>();
    backupBuffer_ = std::make_unique<Buffer>();
    running_ = false;
}

AsyncLogger::~AsyncLogger() {
    if(running_) {
        stop();
    }
}

void AsyncLogger::start() {
    if(running_) return;
    running_ = true;
    file_->open();
    thread_ = std::thread(&AsyncLogger::writeToFile, this);
}

void AsyncLogger::stop() {
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogger::append(const std::string& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    if(data.size() > buffer_->capacity()) {
        std::cerr << "Dropped buffer" << std::endl;
        return;
    }

    if(buffer_->remain() > data.size()) {
        buffer_->append(data);
    } else {
        buffers_.push(std::move(buffer_));
        if(backupBuffer_) {
            buffer_ = std::move(backupBuffer_);     /* 使用备用缓冲区 */
        } else {
            buffer_ = std::make_unique<Buffer>();   /* 创建新缓冲区 */
        }
        buffer_->append(data);
        cond_.notify_one();
    }
}

/* 将缓冲队列写入文件，并重置备用缓冲区 */
void AsyncLogger::flush(BufferQueue& data) {
    while(!data.empty()) {
        auto& buffer = data.front();
        if(file_->filesize() == 0) {
            file_->append(Logger::gHeader.c_str(), Logger::gHeader.size());
        } else if(file_->filesize() > Logger::gMaxFileSize) {
            // TODO: 配置文件化
            file_->rename(Timestamp::now().toFormattedString(false, "%Y-%m-%d %H-%M-%S") + ".log");
            file_.reset(new FileWriter(std::filesystem::current_path(), "log.log"));
            file_->append(Logger::gHeader.c_str(), Logger::gHeader.size());
        }
        file_->append(buffer->data(), buffer->length());
        /* 队列中只有一个缓冲区用于补充，其余丢弃 */
        if(!backupBuffer_) {
            buffer->reset();
            backupBuffer_ = std::move(buffer);  /* 补充备用缓冲区 */
        }
        data.pop();
    }
}

void AsyncLogger::writeToFile() {
    BufferQueue buffersToWrite;
    while(running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            bool avaliable = buffers_.size() > 0;
            if(!avaliable) {
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            while(buffers_.size() > 64) {
                std::cerr << "Too many buffer, drop some" << std::endl;
                buffers_.pop();
            }
            if(backupBuffer_) {
                /* 未使用备用缓冲区，数据在缓冲区上 */
                buffersToWrite.push(std::move(buffer_));
                buffer_ = std::move(backupBuffer_);
            } else {
                /* 缓冲区数据已移入队列，获取数据 */
                buffersToWrite = std::move(buffers_);
            }
        }
        flush(buffersToWrite);
    }
}