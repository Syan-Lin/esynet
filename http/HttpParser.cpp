#include "http/HttpParser.h"

#include <sstream>

#include "http/Request.h"
#include "http/Response.h"

using esynet::http::HttpParser;
using esynet::http::Request;

void getTokens(std::stringstream& ss, std::vector<std::string_view>& tokens, char delim = '\n') {
    size_t start = 0, pos = 0;
    size_t len = ss.view().size();
    const char* str = ss.view().data();

    for(size_t pos = 0; pos < len; pos++) {
        if (str[pos] == delim) {
            tokens.push_back(std::string_view(str + start, pos - start));
            start = pos + 1;
        }
    }
    if(pos > start) {
        tokens.push_back(std::string_view(str + start, pos - start));
    }
}

std::optional<Request> HttpParser::parseRequest(std::string_view message) {
    // 解析请求行
    int pos = message.find("\r\n");
    std::string_view requestLine = message.substr(0, pos);
    std::vector<std::string_view> requestLineTokens;
    std::stringstream line(requestLine.data());
    getTokens(line, requestLineTokens, ' ');

    if(requestLineTokens.size() < 3) {
        return std::nullopt;
    }
    std::string_view method      = requestLineTokens[0];
    std::string_view path        = requestLineTokens[1];
    std::string_view httpVersion = requestLineTokens[2];

    if(method != "GET" && method != "POST") {
        return std::nullopt;
    } else if(httpVersion != "HTTP/1.1" && httpVersion != "HTTP/1.0") {
        return std::nullopt;
    }

    Request request(method, path, httpVersion);

    // 解析请求头
    pos += 2;
    int endPos = message.find("\r\n\r\n");
    std::string_view headers = message.substr(pos, endPos - pos);
    std::vector<std::string_view> headerLines;
    std::stringstream header(headers.data());
    getTokens(header, headerLines, '\r');

    for(int i = 1; i < headerLines.size(); i++) {
        std::string_view headerLine = headerLines[i];
        int colonPos = headerLine.find(":");
        std::string_view key = headerLine.substr(0, colonPos);
        std::string_view value = headerLine.substr(colonPos + 2);
        request.setHeader(key, value);
    }

    // 解析请求体
    int bodyPos = endPos + 4;
    std::string_view body = message.substr(bodyPos);
    request.setBody(body);

    return request;
}