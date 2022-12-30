include_directories(${ESYNET_SOURCE_DIR})

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test net)

add_executable(Timer_test Timer_test.cpp)
target_link_libraries(Timer_test net)

add_test(NAME eventLoop_test COMMAND EventLoop_test)
add_test(NAME timer_test COMMAND Timer_test)