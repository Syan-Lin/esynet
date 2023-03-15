#pragma once

#include <exception>
#include "utils/ErrorInfo.h"

namespace esynet::exception {

class SocketException : public std::exception {
public:
    SocketException(std::string msg, int err): msg_(msg), err_(err) {}
    const int err() { return err_; }
    std::string detail() {
        return "Socket exception(" + errnoStr(err_) + "): " + msg_;
    }
    const char* what() const throw() {
        return "Socket exception";
    }
private:
    std::string msg_;
    const int err_;
};

} /* namespace esynet::exception */