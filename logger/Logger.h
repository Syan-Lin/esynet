#pragma once

/* Standard headers */
#include <filesystem>
#include <mutex>
#include <iostream>
#include <thread>
#include <functional>
#include <unordered_map>

/* Third-party headers */
#include <fmt/format.h>
#include <fmt/chrono.h>

/* Local headers */
#include "utils/Timestamp.h"

namespace esynet::logger {

class SyncLogger;
class AsyncLogger;

}

namespace esynet {

class Logger {
public:
    enum LogLevel { NONE, DEBUG, INFO, WARN, ERROR, FATAL, };
    using BackEndFunction = std::function<void(const std::string& msg)>;
    using FlushFunction = std::function<void()>;
    using StrMap = std::unordered_map<LogLevel, std::string>;

    /* Logger 配置 */
    static const std::string gHeader;
    static const int gMaxFileSize;
    static const LogLevel gLogLevel;
    static fmt::string_view gFormat;
    static fmt::string_view gRename;

    static void setLoggerDefault();
    static void setLogger(BackEndFunction logger);
    template<typename LogBackEnd>
    static void setLogger(LogBackEnd& logger, typename std::enable_if<
                        std::is_same<LogBackEnd, logger::SyncLogger>::value ||
                        std::is_same<LogBackEnd, logger::AsyncLogger>::value>::type* = 0) {
        gSubmitLog = [&logger](const std::string& msg){
            logger.append(msg);
        };
        gAbort = [&logger]{
            logger.abort();
        };
    }

private:
    using Timestamp = utils::Timestamp;
    static BackEndFunction gSubmitLog;
    static FlushFunction gAbort;
    static const StrMap gLogName;
    const std::string& levelToString(LogLevel level) const;

public:
    Logger(const char* file, int line, const char* func, LogLevel);

    template<typename... ARGS>
    void log(const char* fmt, ARGS&&... args) const {
        using namespace fmt;
        if(level_ < gLogLevel) return;

        /* 性能优化 */
        thread_local static std::string thread_id = getThreadId();  /* 每个线程只会执行一次 */
        const std::string& date = getDate(time_);                   /* 缓存日期，避免重复 format */
        std::string time = getTime(time_);                          /* 缓存时间，避免重复 format */

        /* 字段顺序: 日期, 时间, 线程ID, 日志级别, 文件名, 函数名, 行号, 日志信息 */
        const std::string log_info = vformat(gFormat, make_format_args(
                                        date,
                                        time,
                                        thread_id,
                                        levelToString(level_),
                                        file_,
                                        func_,
                                        line_,
                                        vformat(fmt, make_format_args(std::forward<ARGS>(args)...))
                                    ));
        /* 交由后端写入文件 */
        gSubmitLog(log_info);

        if(level_ == FATAL) gAbort();
    }

private:
    auto getThreadId()             const -> std::string;
    auto getDate(const Timestamp&) const -> const std::string&;
    auto getTime(const Timestamp&) const -> std::string;

private:
    int line_;
    const char* file_;
    const char* func_;
    Timestamp time_;
    LogLevel level_;
};

} /* namespace esynet */

#define LOG_DEBUG(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::DEBUG).log(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::INFO ).log(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::WARN ).log(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::ERROR).log(fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::FATAL).log(fmt, ##__VA_ARGS__)