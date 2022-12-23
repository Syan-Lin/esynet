#pragma once

#include <cstddef>
#include <cstring>
#include <string>

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

/* FixedBuffer 性能更好 */
template<int SIZE>
class FixedBuffer {
public:
    FixedBuffer() : cur_(data_) {}
    FixedBuffer(const FixedBuffer&)             = default;
    FixedBuffer(FixedBuffer&&)                  = default;
    FixedBuffer& operator=(const FixedBuffer&)  = default;
    FixedBuffer& operator=(FixedBuffer&&)       = default;
    ~FixedBuffer()                              = default;

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

class Buffer {
public:
    Buffer() = default;
    Buffer(const Buffer&)            = default;
    Buffer(Buffer&&)                 = default;
    Buffer& operator=(const Buffer&) = default;
    Buffer& operator=(Buffer&&)      = default;
    ~Buffer()                        = default;

    void append(StringArg buf) { data_ += std::string(buf.c_str()); }

    const char* data() const { return data_.c_str(); }
    std::string toString() { return data_; }

    size_t length() const { return data_.length(); }
    size_t capacity() const { return data_.max_size(); }
    size_t remain() const { return data_.max_size() - length(); }

    void reset() { data_.clear(); }
    void bzero() { data_ = std::string(char(0), length()); }

private:
    std::string data_;
};
