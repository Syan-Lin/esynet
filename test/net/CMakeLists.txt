include_directories(${ESYNET_SOURCE_DIR})

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test net)

add_executable(EventLoop_wakeup_test EventLoop_wakeup_test.cpp)
target_link_libraries(EventLoop_wakeup_test net)

add_executable(Timer_test Timer_test.cpp)
target_link_libraries(Timer_test net)

add_executable(TimerQueue_test TimerQueue_test.cpp)
target_link_libraries(TimerQueue_test net)

add_test(NAME EventLoop_test COMMAND EventLoop_test)
add_test(NAME EventLoop_wakeup_test COMMAND EventLoop_wakeup_test)
add_test(NAME Timer_test COMMAND Timer_test)
add_test(NAME TimerQueue_test COMMAND TimerQueue_test)