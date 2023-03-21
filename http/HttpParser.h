#pragma once

#include <string_view>
#include <optional>

namespace esynet::http {

class Request;
class Response;

class HttpParser {
public:
    static auto parseRequest(std::string_view message)  -> std::optional<Request>;
};

}; /* namespace esynet */