include_directories(${ESYNET_SOURCE_DIR})

add_executable(Timestamp_Test Timestamp_test.cpp)

add_test(NAME timestamp_test COMMAND Timestamp_Test)