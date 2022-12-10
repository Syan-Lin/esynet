#pragma once

/* System headers */
#include <filesystem>
#include <fstream>
#include <cstring>

/* Local headers */
#include "Buffer.h"

/* 如果文件或目录不存在，则会创建 */
class FileWriter {
public:
    FileWriter(std::filesystem::path path, std::string filename)
            : path_(path / filename) {}
    FileWriter(std::filesystem::path path) : path_(path) {}
    ~FileWriter() {
        if(fileOutput_.is_open()) {
            fileOutput_.close();
        }
    }

    bool exists() { return std::filesystem::exists(path_); }
    std::string filename() { return path_.filename(); }
    int filesize() { return filesize_; }

    void rename(std::string filename) {
        if(!fileOutput_.is_open()) return;
        std::filesystem::rename(path_, path_.parent_path().string() + "/" + filename);
    }

    void open() {
        if(fileOutput_.is_open()) return;

        if(!std::filesystem::exists(path_.parent_path())) {
            std::filesystem::create_directories(path_.parent_path());
        }
        fileOutput_.open(path_.c_str(), std::ios::out | std::ios::app);
        filesize_ = std::filesystem::file_size(path_);
    }

    void append(const char* data, size_t len) {
        if(!fileOutput_.is_open()) open();
        fileOutput_.write(data, len);
        filesize_ += len;
    }

private:
    int filesize_ = -1;
    std::ofstream fileOutput_;
    const std::filesystem::path path_;
};

class FileReader {
public:
    FileReader(std::filesystem::path path, std::string filename)
            : path_(path / filename) {}
    FileReader(std::filesystem::path path) : path_(path) {}
    ~FileReader() {
        if(fileInput_.is_open()) {
            fileInput_.close();
        }
    }

    bool exists() { return std::filesystem::exists(path_); }
    std::string filename() { return path_.filename(); }
    int current() { return current_; }
    int filesize() {
        if(!exists()) return -1;
        if(!fileInput_.is_open()) open();
        return std::filesystem::file_size(path_);
    }

    /* 如果文件不存在，则无行为 */
    void open() {
        if(fileInput_.is_open() || !exists()) return;
        fileInput_.open(path_.c_str(), std::ios::in);
    }

    /* 使用该接口请保证 buf 足够大 */
    void read(char* buf, size_t len) {
        if(!exists()) return;
        if(!fileInput_.is_open()) open();

        fileInput_.read(buf, len);
        current_ += len > filesize() ? filesize() : len;
        if(current_ > filesize()) current_ = filesize();
    }

    std::string read(size_t len = 0) {
        if(!exists()) return "";
        if(!fileInput_.is_open()) open();
        memset(buffer_, 0, sizeof(buffer_));

        if(len > filesize() || len == 0) {
            len = filesize();
        }

        std::string data;
        while(len != 0) {
            if(len < BUFFER_SIZE) {
                memset(buffer_, 0, sizeof(buffer_));
                fileInput_.read(buffer_, len);
                current_ += len;
                len = 0;
            } else {
                fileInput_.read(buffer_, BUFFER_SIZE);
                current_ += BUFFER_SIZE;
                len -= BUFFER_SIZE;
            }
            data += buffer_;
        }
        return data;
    }

private:
    static const int BUFFER_SIZE = 1_MB;
    int current_ = 0;
    char buffer_[BUFFER_SIZE + 1];
    std::ifstream fileInput_;
    const std::filesystem::path path_;
};