cmake_minimum_required(VERSION 3.20)

project(esynet)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

# 是否构建测试文件
set(BUILD_TEST true)

# 是否构建样例文件
set(BUILD_EXAMPLE true)

# 设置库根目录
set(ESYNET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 第三方库引入
# 1. fmt
find_package(fmt REQUIRED QUIET)

# 2. doctest 用于单元测试，非必须
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

# 子目录
add_subdirectory(http)
add_subdirectory(net)
add_subdirectory(logger)
add_subdirectory(example)

# 头文件路径
include_directories(${ESYNET_SOURCE_DIR})

# 作为第三方库
# add_library(esynet)
# target_link_libraries(esynet fmt::fmt logger utils net http)

# 构建样例程序
if(BUILD_EXAMPLE)
    add_executable(main_for_test test.cpp)
    target_link_libraries(main_for_test fmt::fmt logger net http)
    # target_link_libraries(main_for_test utils)
endif()