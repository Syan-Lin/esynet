cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

# 是否构建测试文件
set(BUILD_TEST true)

project(esynet)

# 第三方库引入
# 1. fmt
find_package(fmt REQUIRED QUIET)

# 2. doctest 单元测试用
if(BUILD_TEST)
    enable_testing()
    find_package(doctest REQUIRED QUIET)
    add_subdirectory(test)
endif()

# 第三方库信息
if(doctest_FOUND)
    message(STATUS "doctest found")
endif()
if(fmt_FOUND)
    message(STATUS "fmt found")
endif()

add_subdirectory(logger)

include_directories(utils)
include_directories(logger)

# 非作为第三方库，则编译样例程序
if(CMAKE_PROJECT_NAME STREQUAL "esynet")
    add_executable(main_for_test main.cpp)
    target_link_libraries(main_for_test fmt::fmt)
    target_link_libraries(main_for_test logger)
endif()