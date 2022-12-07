#pragma once

#include <sstream>
#include <chrono>
#include <iomanip>

class Timestamp {
public:
    Timestamp() : microSecondsSinceEpoch_(0) {}
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        : microSecondsSinceEpoch_(microSecondsSinceEpochArg) {}

    Timestamp(const Timestamp&)             = default;
    Timestamp(Timestamp&&)                  = default;
    Timestamp& operator=(const Timestamp&)  = default;
    Timestamp& operator=(Timestamp&&)       = default;
    ~Timestamp()                            = default;

    bool operator<(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_;
    }
    bool operator>(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ > rhs.microSecondsSinceEpoch_;
    }
    bool operator==(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_;
    }
    bool operator!=(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ != rhs.microSecondsSinceEpoch_;
    }
    double operator-(const Timestamp& rhs) const { // return in seconds
        return static_cast<double>(microSecondsSinceEpoch_ - rhs.microSecondsSinceEpoch_) / kMicroSecondsPerSecond;
    }
    Timestamp& operator+(double seconds) {
        microSecondsSinceEpoch_ += static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
        return *this;
    }
    Timestamp& operator+=(double seconds) {
        microSecondsSinceEpoch_ += static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
        return *this;
    }
    Timestamp& operator-(double seconds) {
        microSecondsSinceEpoch_ -= static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
        return *this;
    }
    Timestamp& operator-=(double seconds) {
        microSecondsSinceEpoch_ -= static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
        return *this;
    }

    void swap(Timestamp& that) {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;
    std::string toString() const {
        std::stringstream ss;
        int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
        int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
        ss << seconds << "." << microseconds;
        return ss.str();
    }
    std::string toFormattedString(bool showMicroseconds = true) const {
        using namespace std;

        // Output format
        // reference: https://en.cppreference.com/w/cpp/io/manip/put_time
        const char format[] = "%Y-%m-%d %H:%M:%S";

        time_t time_seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
        stringstream ss;
        ss << put_time(localtime(&time_seconds), format);
        if(showMicroseconds) {
            time_t time_microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
            ss << "." << time_microseconds;
        }
        return ss.str();
    }

    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    static Timestamp now() {
        using namespace std::chrono;
        return Timestamp(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
    }

private:
    int64_t microSecondsSinceEpoch_;
};