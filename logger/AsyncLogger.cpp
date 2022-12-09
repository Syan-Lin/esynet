#include "PerformanceAnalyser.h"
#include "AsyncLogger.h"

// TODO: 配置文件化
const std::string AsyncLogger::gHeader = "Date                       Tid  Level File            Function             Line  Msg\n";
const int AsyncLogger::gMaxFileSize = 100 MB;
// inline const std::string Logfile::gHeader = fmt::format("{:26} {:4} {:5} {:15} {:20} {:5} {}\n",
//                                                     "Date", "Tid", "Level", "File", "Function", "Line", "Msg");

TimeAnalyser ta;
TimeStatistics ts;

AsyncLogger::AsyncLogger(std::filesystem::path path, int flushInterval) :
        file_(new FileWriter(path)), flushInterval_(flushInterval)  {
    buffer_ = std::make_unique<Buffer>();
    backupBuffer_ = std::make_unique<Buffer>();
    running_ = false;

    /* 性能测试 */
    // ta.addStat(std::bind(
    //     &TimeStatistics::stat, &ts,
    //     std::placeholders::_1, std::placeholders::_2
    // ));
    // ta.start("AsyncLogger");
}

AsyncLogger::~AsyncLogger() {
    if(running_) {
        stop();
    }

    /* 性能测试 */
    // ta.stop("AsyncLogger");
    dbg(ts.toString(TimeAnalyser::SECONDS));
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
    // ta.start("hand_to_backend");
    // ta.start("append_lock");
    std::unique_lock<std::mutex> lock(mutex_);
    // ta.stop("append_lock");
    // ta.start("append");
    if(data.size() > buffer_->capacity()) {
        // TODO: 错误处理
        return;
    }

    if(buffer_->remain() > data.size()) { // 缓冲区足够大，直接写入
        //dbg("(缓冲区足够大，直接写入)");
        buffer_->append(data);
    } else {
        buffers_.push(std::move(buffer_));
        if(backupBuffer_) { // 使用备用缓冲区
            //dbg("(使用备用缓冲区)");
            buffer_ = std::move(backupBuffer_);
        } else { // 备用缓冲区也已经使用，只能创建新的缓冲区
            //dbg "(备用缓冲区也已经使用，只能创建新的缓冲区)");
            buffer_ = std::make_unique<Buffer>();
        }
        buffer_->append(data);
        //dbg("(唤醒写线程，将数据写入文件)");
        cond_.notify_one(); // 唤醒写线程，将数据写入文件
    }

    // ta.stop("append");
    // ta.stop("hand_to_backend");
}

void AsyncLogger::flush(BufferQueue& data) {
    //dbg("(写入文件)");
    //dbgdata.size());
    // ta.start("flush");
    while(!data.empty()) {
        auto& buffer = data.front();
        if(file_->filesize() == 0) { // 新文件，添加 Header
            file_->append(gHeader.c_str(), gHeader.size());
        } else if(file_->filesize() > gMaxFileSize) { // 单文件过大，新建文件
            // TODO: 配置文件化
            file_->rename(Timestamp::now().toFormattedString(false, "%Y-%m-%d %H-%M-%S") + ".log");
            file_.reset(new FileWriter(std::filesystem::current_path(), "log.log"));
            file_->append(gHeader.c_str(), gHeader.size());
        }
        file_->append(buffer->data(), buffer->length());
        if(!backupBuffer_) { // 补充备用缓冲区
            //dbg("(补充备用缓冲区)");
            buffer->reset();
            backupBuffer_ = std::move(buffer);
        }
        data.pop();
    }
    // ta.stop("flush");
}

void AsyncLogger::writeToFile() {
    BufferQueue buffersToWrite;
    while(running_) {
        {
            // ta.start("lock_time");
            std::unique_lock<std::mutex> lock(mutex_);

            bool avaliable = buffers_.size() > 0;
            if(!avaliable) { // 队列中无数据，等待
                //dbg("(队列中无数据，等待)");
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            // ta.stop("lock_time");
            while(buffers_.size() > 64) { // 缓冲区太多数据，丢弃一部分
                dbg("(缓冲区太多数据，丢弃一部分)");
                buffers_.pop();
            }
            // ta.start("writeToFile");
            if(backupBuffer_) { // 还没使用备用缓冲区，说明数据全在 buffer_ 上，需要获取
                //dbg("(还没使用备用缓冲区，将 buffer_ 写入文件)");
                buffersToWrite.push(std::move(buffer_));
                buffer_ = std::move(backupBuffer_);
            } else { // 使用了备用缓冲区，则数据写入了队列，此时 buffer_ 用于写入，不覆盖
                //dbg("(使用了备用缓冲区，将队列写入文件，不将 buffer_ 写入文件)");
                buffersToWrite = std::move(buffers_);
            }
            // ta.stop("writeToFile");
        }
        // 将缓冲区中的数据写入文件，重置备用缓冲，多余的缓存则丢弃
        flush(buffersToWrite);
    }
}