#pragma once

/* System headers */
#include <filesystem>
#include <mutex>
#include <iostream>
#include <thread>
#include <functional>

/* Third-party headers */
#include <fmt/format.h>
#include <unordered_map>

/* Local headers */
#include "Timestamp.h"
#include "PerformanceAnalyser.h"
#include "Singleton.h"

/* 用户所使用的 Logger 前端，具体 Log 任务通过 BackEndFunction 传递给后端
 * 通过宏使用，每次都会创建新对象，所以不用保证线程安全 */
class Logger {
public:
    enum LogLevel { NONE, DEBUG, INFO, WARN, ERROR, FATAL, };
    using BackEndFunction = std::function<void(const std::string& msg)>;
    using StrMap = std::unordered_map<LogLevel, std::string>;
    /* 设置 Logger 后端 */
    static void setLogger(BackEndFunction);
    /* 默认级别: INFO */
    static void setLogLevel(LogLevel);

private:
    static LogLevel gLogLevel;
    static BackEndFunction gLogToFile;
    static const StrMap gLogName;
    const std::string& levelToString(LogLevel level);
    thread_local static char* thread_id_;

    // struct ComplieTimeString {
    //     template<int N>
    //     ComplieTimeString(const char (&arr)[N]) : data_(arr), size_(N-1) {}
    //     const char* data_;
    //     int size_;
    // };

public:
    Logger(const char*, int, const char*, LogLevel);

    /* 模板函数，必须放在头文件中 */
    template<typename... ARGS>
    void log(fmt::string_view fmt, ARGS&&... args) {
        using namespace fmt;
        if(level_ < gLogLevel) return;

        // Singleton<TimeAnalyser>::instance().start("get_thread_id");

        thread_local static std::string thread_id = getThreadId();  /* TODO: 待测试 */

        // Singleton<TimeAnalyser>::instance().stop("get_thread_id");

        // Singleton<TimeAnalyser>::instance().start("format_info");
        /* TODO: 优化性能 */
        /* [time: 26] | [tid: 4] | [mode: 5] | [filename: 15] | [func: 20] | [line: 5] */
        const std::string log_info = "Date                       Tid  Level File            Function             Line  Msg\n";
        // std::string log_info = format("{:26} {:4} {:5} {:15} {:20} {:<5} {}\n",
        //                                 time_.toFormattedString(),
        //                                 thread_id, levelToString(level_), file_, func_, line_,
        //                                 vformat(fmt, make_format_args(std::forward<ARGS>(args)...)));

        // Singleton<TimeAnalyser>::instance().stop("format_info");

        // Singleton<TimeAnalyser>::instance().start("hand_to_backend");
        /* 交由后端写入文件 */
        gLogToFile(log_info);
        // Singleton<TimeAnalyser>::instance().stop("hand_to_backend");
    }

private:
    const std::string getThreadId();

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