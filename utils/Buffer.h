#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include "StringPiece.h"

constexpr int operator""_B(unsigned long long num) {
    return num;
}
constexpr int operator""_KB(unsigned long long num) {
    return num * 1024;
}
constexpr int operator""_MB(unsigned long long num) {
    return num * 1024 * 1024;
}
constexpr int operator""_GB(unsigned long long num) {
    return num * 1024 * 1024 * 1024;
}

namespace esynet::utils {

template<int SIZE>
class LogBuffer {
public:
    LogBuffer() : cur_(data_) {}
    LogBuffer(const LogBuffer&)             = default;
    LogBuffer(LogBuffer&&)                  = default;
    LogBuffer& operator=(const LogBuffer&)  = default;
    LogBuffer& operator=(LogBuffer&&)       = default;
    ~LogBuffer()                              = default;

    /* 在 append 之前请保证缓冲区足够大 */
    void append(StringArg buf) {
        size_t len = strlen(buf.c_str());
        if (remain() > len) {
            memcpy(cur_, buf.c_str(), len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    std::string toString() { return std::string(data(), length()); }

    size_t length() const { return cur_ - data_; }
    size_t capacity() const { return SIZE; }
    size_t remain() const { return data_ + sizeof(data_) - cur_; }

    void reset() { cur_ = data_; }
    void bzero() { ::bzero(data_, sizeof(data_)); }

private:
    char data_[SIZE];
    char* cur_;
};

/* TODO: Buffer for net */
class Buffer {

};

} /* namespace esynet::utils */