#include "logger/Logger.h"

/* Local headers */
#include "utils/FileUtil.h"

using esynet::Logger;

static void logToConsole(const std::string& msg) {
    static bool first = true;
    if(first) {
        std::cout << Logger::gHeader;
        first = false;
    }
    std::cout << msg;
}

/* Logger 配置 */
const Logger::LogLevel  Logger::gLogLevel    = Logger::LogLevel::DEBUG;
const std::string       Logger::gHeader      = "Date       Time            Tid  Level File            Function             Line  Msg\n";
const int               Logger::gMaxFileSize = 100_MB;
fmt::string_view        Logger::gFormat      = "{:10} {:15} {:4} {:5} {:15} {:20} {:<5} {}\n";
Logger::BackEndFunction Logger::gSubmitLog   = std::bind(logToConsole, std::placeholders::_1);
Logger::FlushFunction   Logger::gAbort       = []{ abort(); };
const Logger::StrMap    Logger::gLogName     = {
    {Logger::NONE, "None"},
    {Logger::DEBUG, "Debug"},
    {Logger::INFO, "Info"},
    {Logger::WARN, "Warn"},
    {Logger::ERROR, "Error"},
    {Logger::FATAL, "Fatal"}
};

void Logger::setLogger(BackEndFunction func) { gSubmitLog = func; }
void Logger::setLoggerDefault() { gSubmitLog = std::bind(logToConsole, std::placeholders::_1); }
const std::string& Logger::levelToString(LogLevel level) const {
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

Logger::Logger(const char* file, int line, const char* func, LogLevel level):
            line_(line), func_(func), level_(level) {
    file_ = strrchr(file, '/') + 1;
    time_ = Timestamp::now();
}

/* 线程数超过300时，线程ID后四位有可能发生重复 */
std::string Logger::getThreadId() const {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    thread_id = thread_id.substr(thread_id.length() - 4, 4);
    return thread_id;
}

const std::string& Logger::getDate(const Timestamp& time) const {
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

std::string Logger::getTime(const Timestamp& time) const {
    static time_t lastSecond = 0;
    static std::string secondStr = "";
    int64_t microseconds = time.microSecondsSinceEpoch();
    time_t seconds = microseconds / Timestamp::kMicroSecondsPerSecond;
    if(seconds != lastSecond) {
        lastSecond = seconds;
        secondStr = fmt::format("{:%H:%M:%S}.", fmt::localtime(seconds));
    }
    microseconds %= Timestamp::kMicroSecondsPerSecond;
    return secondStr + fmt::format("{:06d}", microseconds);
}