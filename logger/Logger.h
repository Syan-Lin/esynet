#pragma once

/* System headers */
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
#include "Timestamp.h"
#include "FileUtil.h"

/* 用户所使用的 Logger 前端，具体 Log 任务通过 BackEndFunction 传递给后端
 * 通过宏使用，每次都会创建新对象，所以不用保证线程安全 */
class Logger {
public:
    enum LogLevel { NONE, DEBUG, INFO, WARN, ERROR, FATAL, };
    using BackEndFunction = std::function<void(const std::string& msg)>;
    using StrMap = std::unordered_map<LogLevel, std::string>;
    static void setLogger(BackEndFunction);
    static void setLogLevel(LogLevel);
    static const std::string gHeader;
    static const int gMaxFileSize;

private:
    static LogLevel gLogLevel;
    static BackEndFunction gSubmitLog;
    static const StrMap gLogName;
    static fmt::string_view gFormat;
    const std::string& levelToString(LogLevel level);

public:
    Logger(const char*, int, const char*, LogLevel);

    /* 模板函数，必须放在头文件中 */
    template<typename... ARGS>
    void log(const char* fmt, ARGS&&... args) {
        using namespace fmt;
        if(level_ < gLogLevel) return;

        /* 性能优化相关 */
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
    }

private:
    std::string getThreadId();
    const std::string& getDate(Timestamp&);
    std::string getTime(Timestamp&);
    void loadConfig();

private:
    Timestamp time_;
    const char* file_;
    const char* func_;
    int line_;
    LogLevel level_;
};

#define LOG_DEBUG(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::DEBUG).log(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::INFO ).log(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::WARN ).log(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::ERROR).log(fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::FATAL).log(fmt, ##__VA_ARGS__)