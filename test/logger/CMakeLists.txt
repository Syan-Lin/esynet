include_directories(${PROJECT_SOURCE_DIR}/logger)

add_executable(Timestamp_Test Timestamp_test.cpp)

add_test(NAME timestamp_test COMMAND Timestamp_Test)