#pragma once

#include <exception>
#include "utils/ErrorInfo.h"

namespace esynet::exception {

class NetworkException : public std::exception {
public:
    NetworkException(std::string msg, int err): msg_(msg), err_(err) {}
    const int err() { return err_; }
    std::string detail() {
        return "Network exception(" + errnoStr(err_) + "): " + msg_;
    }
    const char* what() const throw() {
        return "Network exception";
    }
private:
    std::string msg_;
    const int err_;
};

} /* namespace esynet::exception */