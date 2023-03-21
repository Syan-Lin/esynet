#pragma once

#include <string_view>
#include <optional>

#include "utils/NonCopyable.h"
#include "http/Header.h"

namespace esynet::http {

class Response {
public:
    template<typename T>
    auto header(std::string_view key) const -> std::optional<T>;
    auto param(std::string_view key)  const -> std::optional<std::string_view>;
    void setHeader(std::string_view key, std::string_view value);
    void setRedirect(std::string_view location, int status = 302);
    void setContent(std::string_view body);
    void setContent(const char* body, size_t len);
    int  paramCount() const;

private:
    int status_;
    std::string version_;
    std::string location_;
    std::string body_;
    Header header_;

};

}; /* namespace esynet::http */