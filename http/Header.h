#pragma once

#include <string_view>
#include <vector>
#include <map>

#include "utils/StringPiece.h"
#include "http/ConceptHelper.h"

namespace esynet::http {

class Header {
private:
    using ValueMap = std::multimap<std::string, std::string>;
    using IterAndSize = std::pair<ValueMap::const_iterator, size_t>;

public:
    Header() = default;
    ~Header() = default;

    void set(std::string_view key, IntegerType auto value) {
        values_.emplace(key, std::to_string(value));
    }
    void set(std::string_view key, FloatingPointType auto value) {
        values_.emplace(key, std::to_string(value));
    }
    void set(std::string_view key, BooleanType auto value) {
        if(value)
            values_.emplace(key, "true");
        else
            values_.emplace(key, "false");
    }
    void set(std::string_view key, StringType auto value) {
        values_.emplace(key, value);
    }
    void set(std::string_view key, SequentialContainerType auto value) {
        std::string str;
        str += "[";
        for (const auto& elem : value) {
            if constexpr(std::is_same_v<decltype(elem), const std::string&>) {
                str += elem + ",";
            } else {
                str += std::to_string(elem) + ",";
            }
        }
        str.back() = ']';
        values_.emplace(key, str);
    }
    void set(std::string_view key, MapType auto value) {
        std::string str;
        str += "{";
        for (const auto& [k, v] : value) {
            if constexpr(std::is_same_v<decltype(k), const std::string>) {
                str += k + ":";
            } else {
                str += std::to_string(k) + ":";
            }
            if constexpr(std::is_same_v<decltype(v), const std::string>) {
                str += v + ",";
            } else {
                str += std::to_string(v) + ",";
            }
        }
        str.back() = '}';
        values_.emplace(key, str);
    }
    void set(std::string_view key, SetType auto value) {
        std::string str;
        str += "{";
        for (const auto& elem : value) {
            if constexpr(std::is_same_v<decltype(elem), const std::string&>) {
                str += elem + ",";
            } else {
                str += std::to_string(elem) + ",";
            }
        }
        str.back() = '}';
        values_.emplace(key, str);
    }

    auto get(std::string_view key) -> std::optional<IterAndSize> {
        auto iter  = values_.find(key.data());
        int  count = values_.count(key.data());
        if(count == 0) {
            return std::nullopt;
        } else {
            return IterAndSize{iter, count};
        }
    }

private:
    ValueMap values_;
};

}; /* namespace esynet::http */