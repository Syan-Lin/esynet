include_directories(${ESYNET_SOURCE_DIR})

add_executable(Header_test Header_test.cpp)
target_link_libraries(Header_test http)

add_test(NAME Header_test COMMAND Header_test)