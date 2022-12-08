#pragma once

// System headers
#include <filesystem>
#include <mutex>
#include <iostream>
#include <thread>

// Third-party headers
#include <fmt/format.h>

// Local headers
#include "Timestamp.h"
#include "Logfile.h"

// front-end of logger system, users use this to log messages
// macro creates Logger object every time, so it is not need to be thread safe
class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };
    std::string levelToString(LogLevel level) {
        switch(level) {
            case DEBUG: return "DEBUG"; break;
            case INFO:  return "INFO";  break;
            case WARN:  return "WARN";  break;
            case ERROR: return "ERROR"; break;
            case FATAL: return "FATAL"; break;
        }
        return "";
    }
    static LogLevel gLogLevel;
    static std::unique_ptr<Logfile> gLogfile;

public:
    Logger(const char* file, int line, const char* func, LogLevel level)
            : line_(line), func_(func), level_(level) {
        file_ = std::string(strrchr(file, '/') + 1);
        time_ = Timestamp::now();
    }

    template<typename... ARGS>
    void log(fmt::string_view fmt, ARGS&&... args) {
        if(level_ < gLogLevel) return;

        using namespace fmt;

        std::stringstream ss;
        ss << std::this_thread::get_id();
        std::string thread_id = ss.str();
        thread_id = thread_id.substr(thread_id.length() - 4, 4);

        // TODO: 优化性能
        // [time: 26] | [tid: 4] | [mode: 5] | [filename: 15] | [func: 20] | [line: 5]
        std::string log_info = format("{:26} {:4} {:5} {:15} {:20} {:<5} {}\n",
                                        time_.toFormattedString(),
                                        thread_id, levelToString(level_), file_, func_, line_,
                                        vformat(fmt, make_format_args(std::forward<ARGS>(args)...)));

        // send data to back-end
        gLogfile->append(log_info.c_str(), log_info.size());
    }

private:
    Timestamp time_;
    std::string file_;
    std::string func_;
    int line_;
    LogLevel level_;
};

#define LOG_DEBUG(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::DEBUG).log(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::INFO).log(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::WARN).log(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::ERROR).log(fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger(__FILE__, __LINE__, __func__, Logger::LogLevel::FATAL).log(fmt, ##__VA_ARGS__)

// TODO: 配置文件化
inline Logger::LogLevel Logger::gLogLevel = Logger::LogLevel::DEBUG;
inline std::unique_ptr<Logfile> Logger::gLogfile = std::make_unique<Logfile>(std::filesystem::current_path()/"log.log", 1);

// starts log thread
inline void startLog() {
    Logger::gLogfile->start();
}