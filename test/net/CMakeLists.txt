include_directories(${ESYNET_SOURCE_DIR})

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test net)

add_test(NAME eventLoop_test COMMAND EventLoop_test)