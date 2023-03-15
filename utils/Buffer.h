#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <vector>
#include <sys/uio.h>

#include "StringPiece.h"
#include "logger/Logger.h"
#include "net/base/Socket.h"
#include "exception/SocketException.h"

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
    ~LogBuffer()                            = default;

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

    bool empty()        const { return cur_ == data_; }
    size_t length()     const { return cur_ - data_; }
    size_t capacity()   const { return SIZE; }
    size_t remain()     const { return data_ + sizeof(data_) - cur_; }

    void reset() { cur_ = data_; }
    void bzero() { ::bzero(data_, sizeof(data_)); }

private:
    char data_[SIZE];
    char* cur_;
};

/* Buffer class model： 非线程安全
 *
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 *
 */

class Buffer {
public:
    static const size_t kCheapPrepend = 8_B;
    static const size_t kInitialSize = 1_KB;

    Buffer(size_t initSize = kInitialSize)
            : buffer_(kCheapPrepend + initSize),
              readerIndex_(kCheapPrepend),
              writerIndex_(kCheapPrepend) {}

    void swap(Buffer& rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    const char* findCRLF() const {
        std::string CRLF = "\r\n";
        const char* crlf = std::search(beginRead(), endRead(), CRLF.begin(), CRLF.end());
        return crlf == endRead() ? nullptr : crlf;
    }
    const char* findCRLF(const char* start) const {
        checkStart(start);
        std::string CRLF = "\r\n";
        const char* crlf = std::search(start, endRead(), CRLF.begin(), CRLF.end());
        return crlf == endRead() ? nullptr : crlf;
    }
    const char* findEOL() const {
        std::string EOL = "\n";
        const char* eol = std::search(beginRead(), endRead(), EOL.begin(), EOL.end());
        return eol == endRead() ? nullptr : eol;
    }
    const char* findEOL(const char* start) const {
        checkStart(start);
        std::string EOL = "\n";
        const char* eol = std::search(start, endRead(), EOL.begin(), EOL.end());
        return eol == endRead() ? nullptr : eol;
    }
    const char* find(StringPiece target) const {
        const char* pos = std::search(beginRead(), endRead(), target.begin(), target.end());
        return pos == endRead() ? nullptr : pos;
    }
    const char* find(StringPiece target, const char* start) const {
        checkStart(start);
        const char* pos = std::search(start, endRead(), target.begin(), target.end());
        return pos == endRead() ? nullptr : pos;
    }

    using ReadableRange = std::pair<const char*, const char*>;
    ReadableRange readRange() { return { beginRead(), endRead()}; }
    /* 获取数据头指针 */
    const char* beginRead() const { return begin() + readerIndex_; }
    /* 获取数据尾后指针 */
    const char* endRead() const { return begin() + writerIndex_; }

    /* 读取数据，且移动读指针 */
    int8_t readByte() {
        int8_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        retrieve(sizeof(result));
        return result;
    }
    int16_t readByte2() {
        int16_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        retrieve(sizeof(result));
        return result;
    }
    int32_t readByte4() {
        int32_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        retrieve(sizeof(result));
        return result;
    }
    int64_t readByte8() {
        int64_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        retrieve(sizeof(result));
        return result;
    }

    /* 读取数据，但不移动读指针 */
    int8_t peekByte() const {
        int8_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        return result;
    }
    int16_t peekByte2() const {
        int16_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        return result;
    }
    int32_t peekByte4() const {
        int32_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        return result;
    }
    int64_t peekByte8() const {
        int64_t result;
        std::copy(beginRead(), beginRead() + sizeof(result), &result);
        return result;
    }

    /* 移动数据头指针，代表真正取出数据，数据头指针之前可随时被覆盖 */
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }
    void retrieveUntil(const char* end) {
        if(end >= beginRead() && end <= endRead()) {
            retrieve(end - beginRead());
        } else {
            LOG_ERROR("end out of bound(end: {:p})", static_cast<const void*>(end));
        }
    }
    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }
    std::string retrieveAsString(size_t len) {
        if(len <= readableBytes()) {
            std::string result(beginRead(), len);
            retrieve(len);
            return result;
        } else {
            LOG_ERROR("len out of bound(len: {})", len);
            return "";
        }
    }

    /* 移动写数据指针 */
    void undoAppend(size_t len) {
        if(len <= readableBytes()) {
            writerIndex_ -= len;
        } else {
            LOG_ERROR("undoWrite failed(len: {})", len);
        }
    }
    void append(const StringPiece& str) { append(str.data(), str.size()); }
    void append(const void* data, size_t len) {
        const char* dataBytes = static_cast<const char*>(data);
        if (writableBytes() < len) {
            makeSpace(len);
        }
        std::copy(dataBytes, dataBytes + len, beginWrite());
        if(len <= writableBytes()) {
            writerIndex_ += len;
        } else {
            LOG_ERROR("no enough space for append(len: {})", len);
        }
    }

    /* 预留前缀信息 */
    void prepend(const void* data, size_t len) {
        if(len > prependableBytes()) {
            LOG_FATAL("prepend out of bound(len: {})", len);
        }
        readerIndex_ -= len;
        const char* dataBytes = static_cast<const char*>(data);
        std::copy(dataBytes, dataBytes + len, beginPrepend());
    }

    /* 从Socket中读取数据，错误时会将error设为值，水平触发不用担心一次读取不完的问题
     * 如果是边沿触发，则需要考虑 */
    size_t readSocket(Socket sock) {
        char extraBuf[64_KB];
        struct iovec vec[2];
        const size_t writable = writableBytes();
        vec[0].iov_base = begin() + writerIndex_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extraBuf;
        vec[1].iov_len = sizeof extraBuf;

        /* Buffer空间足够时，不会使用extraBuf，当空间不够时，才使用extraBuf，并在
         * 后续填充至Buffer中，这样做的理由是只需要一次系统调用就可以获得足够大的
         * 数据，而不必预先在Buffer中预留大量的空间来准备可能（很少）来临的大数据 */
        ssize_t n = sock.readv(vec, 2);
        if (n <= writable) {
            writerIndex_ += n;
        } else {
            writerIndex_ = buffer_.size();
            append(extraBuf, n - writable);
        }
        // if (n < 0) {
        //     error = errno;
        // } else if (n <= writable) {
        //     writerIndex_ += n;
        // } else {
        //     writerIndex_ = buffer_.size();
        //     append(extraBuf, n - writable);
        // }
        return n;
    }

private:
    const char* begin() const { return &*buffer_.begin(); }
    char* begin() { return &*buffer_.begin(); }
    char* beginWrite() { return begin() + writerIndex_; }
    char* beginPrepend() { return begin() + readerIndex_; }
    /* 检查start指针合法性,非法会abort程序 */
    void checkStart(const char* start) const {
        if(start >= endRead() || start < beginRead()) {
            LOG_FATAL("start out of bound(start: {:p})", static_cast<const void*>(start));
        }
    }
    /* 在当前数据内部腾挪出空间 */
    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_+len);
        } else {
            /* 将数据往前移 */
            size_t readable = readableBytes();
            std::copy(beginRead(), endRead(), begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

} /* namespace esynet::utils */