#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <chrono>
#include <iomanip>
#include <fmt/format.h>

namespace esynet::utils {

/* 非线程安全，频繁计时有一定的性能损失，每次调用约有 数十纳秒 的开销 */
class TimeAnalyser {
public:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using TimePointMap = std::map<const char*, TimePoint>;
    using TimeMap = std::map<const char*, int64_t>;
    using TimeStatistics = std::function<void(const char*, int64_t)>;
    enum Unit { NANOSECONDS, MICROSECONDS, MILLISECONDS, SECONDS };

    static double toMicroseconds(int64_t nanoSeconds) {
        return static_cast<double>(nanoSeconds / 1000.0);
    }
    static double toMilliSeconds(int64_t nanoSeconds) {
        return static_cast<double>(nanoSeconds / 1000.0 / 1000.0);
    }
    static double toSeconds(int64_t nanoSeconds) {
        return static_cast<double>(nanoSeconds / 1000.0 / 1000.0 / 1000.0);
    }
    static std::string toString(int64_t nanoSeconds, Unit unit = MICROSECONDS) {
        using fmt::format;
        switch(unit) {
            case NANOSECONDS:
                return format("{} ns", nanoSeconds);
            case MICROSECONDS:
                return format("{:.3f} μs", toMicroseconds(nanoSeconds));
            case MILLISECONDS:
                return format("{:.3f} ms", toMilliSeconds(nanoSeconds));
            case SECONDS:
                return format("{:.3f} s", toSeconds(nanoSeconds));
        }
        return "";
    }

public:
    TimeAnalyser() {
        addToStat_ = std::bind(&TimeAnalyser::doNothing, this, std::placeholders::_1, std::placeholders::_2);
    }
    void start(const char* name) {
        timePointMap_[name] = now();
    }
    void stop(const char* name) {
        using namespace std::chrono;
        if(timePointMap_.find(name) == timePointMap_.end()) return;
        timeMap_[name] = duration_cast<nanoseconds>(now() - timePointMap_[name]).count();
        addToStat_(name, timeMap_[name]);
    }
    int64_t get(const char* name) {
        if(timeMap_.find(name) != timeMap_.end()) {
            return timeMap_[name];
        }
        return -1;
    }
    template<typename Function>
    int64_t func(Function function) {
        using namespace std::chrono;
        TimePoint start = now();
        function();
        return duration_cast<nanoseconds>(now() - start).count();
    }

    std::string toString(Unit unit = MICROSECONDS) {
        std::string result;
        for(auto& [name, time] : timeMap_) {
            result += fmt::format("name: {:s}, time: {:s}\n", name, toString(time, unit));
        }
        return result;
    }

    TimeMap& getMap() { return timeMap_; }
    void addStat(TimeStatistics stat) { addToStat_ = stat; }

private:
    TimePoint now() { return std::chrono::high_resolution_clock::now(); }
    void doNothing(const char*, int64_t) {}

private:
    TimeMap timeMap_;
    TimePointMap timePointMap_;
    TimeStatistics addToStat_;
};

class TimeStatistics {
public:
    using TimeMap = TimeAnalyser::TimeMap;

public:
    void stat(const char* name, int64_t time) {
        if(tm_.find(name) == tm_.end()) {
            tm_[name] = 0;
        }
        tm_[name] += time;
    }
    TimeMap& getMap() { return tm_; }
    std::string toString(TimeAnalyser::Unit unit = TimeAnalyser::MILLISECONDS) {
        std::string result;
        for(auto& [name, time] : tm_) {
            result += fmt::format("id: {:s}, time: {:s}\n", name, TimeAnalyser::toString(time, unit));
        }
        return result;
    }

private:
    TimeMap tm_;
};

/* TODO: 内存分析 */
class MemoryAnalyser {

};

} /* namespace esynet::utils */