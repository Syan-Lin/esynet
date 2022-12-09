#include "Logger.h"

static void doNothing(const std::string&) {}

/* TODO: 配置文件化 */
Logger::LogLevel Logger::gLogLevel = Logger::LogLevel::INFO;
Logger::BackEndFunction Logger::gLogToFile = std::bind(doNothing, std::placeholders::_1);
const Logger::StrMap Logger::gLogName = {
    {Logger::NONE, "None"},
    {Logger::DEBUG, "Debug"},
    {Logger::INFO, "Info"},
    {Logger::WARN, "Warn"},
    {Logger::ERROR, "Error"},
    {Logger::FATAL, "Fatal"}
};

void Logger::setLogger(BackEndFunction func) {
    gLogToFile = func;
}

void Logger::setLogLevel(LogLevel level) {
    gLogLevel = level;
}

const std::string& Logger::levelToString(LogLevel level) {
    switch(level) {
        case DEBUG: return gLogName.at(DEBUG);
        case INFO:  return gLogName.at(INFO);
        case WARN:  return gLogName.at(WARN);
        case ERROR: return gLogName.at(ERROR);
        case FATAL: return gLogName.at(FATAL);
        default: break;
    }
    return Logger::gLogName.at(NONE);
}

Logger::Logger(const char* file, int line, const char* func, LogLevel level)
        : line_(line), func_(func), level_(level) {
    file_ = strrchr(file, '/') + 1;
    time_ = Timestamp::now();
    // Singleton<TimeAnalyser>::instance().stop("init_obj");
}

#include "dbg.h"

std::string Logger::getThreadId() {
    dbg("getThreadId");
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    thread_id = thread_id.substr(thread_id.length() - 4, 4);
    return thread_id;
}

const std::string& Logger::getDate(Timestamp& time) {
    static time_t lastDay = 0;
    static std::string dateStr = "";
    time_t seconds = time.secondsSinceEpoch();
    time_t day = seconds / 86400;
    if(day != lastDay) {
        lastDay = day;
        dateStr = fmt::format("{:%Y-%m-%d}", fmt::localtime(seconds));
    }
    return dateStr;
}

std::string Logger::getTime(Timestamp& time) {
    static time_t lastSecond = 0;
    static std::string secondStr = "";
    time_t seconds = time.secondsSinceEpoch();
    if(seconds != lastSecond) {
        lastSecond = seconds;
        secondStr = fmt::format("{:%H:%M:%S}", fmt::localtime(seconds));
    }
    int64_t microseconds = time.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond;
    return secondStr + fmt::format(".{:06d}", microseconds);
}