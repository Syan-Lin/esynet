#pragma once

#include <string_view>
#include <optional>

#include "utils/NonCopyable.h"
#include "http/Header.h"

namespace esynet::http {

class Request {
public:
    Request(std::string_view method, std::string_view path, std::string_view version);

    template<typename T>
    auto header(std::string_view key) const -> std::optional<T>;
    auto param(std::string_view key)  const -> std::optional<std::string_view>;
    void setHeader(std::string_view key, std::string_view value);
    void setBody(std::string_view body);
    int  paramCount() const;

private:
    std::string_view method_;
    std::string_view path_;
    std::string_view version_;
    std::string_view body_;
    Header header_;
};

}; /* namespace esynet::http */