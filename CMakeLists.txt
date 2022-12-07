cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

project(CodeSnippets)

add_subdirectory(thirdparty/fmt)

include_directories(thirdparty)

add_executable(CodeSnippets main.cpp)

target_link_libraries(CodeSnippets fmt::fmt)