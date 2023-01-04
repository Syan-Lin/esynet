#pragma once

namespace esynet::utils {

class NonCopyable {
public:
    NonCopyable(const NonCopyable&)    = delete;
    void operator=(const NonCopyable&) = delete;
protected:
    NonCopyable()  = default;
    ~NonCopyable() = default;
};

} /* namespace esynet::utils */