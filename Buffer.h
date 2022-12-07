#pragma once

#include <cstddef>
#include <cstring>
#include <string>

// FixedBuffer should have a better performance if used properly.
template<int SIZE>
class FixedBuffer {
public:
    FixedBuffer() : cur_(data_) {}
    FixedBuffer(const FixedBuffer&)             = default;
    FixedBuffer(FixedBuffer&&)                  = default;
    FixedBuffer& operator=(const FixedBuffer&)  = default;
    FixedBuffer& operator=(FixedBuffer&&)       = default;
    ~FixedBuffer()                              = default;

    // Should check remaining space before calling append
    void append(const char* buf, size_t len) {
        if (remain() > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }
    void append(const std::string& buf) {
        if (remain() > buf.length()) {
            memcpy(cur_, buf.c_str(), buf.length());
            cur_ += buf.length();
        }
    }

    const char* data() const { return data_; }
    std::string toString() { return std::string(data_, length()); }

    int length() const { return static_cast<int>(cur_ - data_); }
    int capacity() const { return SIZE; }
    int remain() const { return static_cast<int>(data_ + sizeof(data_) - cur_); }

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

    void append(const char* buf, size_t len) { data_ += std::string(buf, len); }
    void append(const std::string& buf) { data_ += buf; }

    const char* data() const { return data_.c_str(); }
    std::string toString() { return data_; }

    int length() const { return data_.length(); }
    int capacity() const { return data_.max_size(); }
    int remain() const { return data_.max_size() - length(); }

    void reset() { data_.clear(); }
    void bzero() { data_ = std::string(char(0), length()); }

private:
    std::string data_;
};
