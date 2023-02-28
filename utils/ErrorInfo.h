#pragma once

#include <string.h>
#include <string>

/* 线程安全 */
inline std::string errnoStr(int err) {
    char error_info[64];
    strerror_r(err, error_info, sizeof error_info);
    return std::string(error_info);
}