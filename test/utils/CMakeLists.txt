include_directories(${ESYNET_SOURCE_DIR})

add_executable(FileUtil_Test FileUtil_test.cpp)
add_executable(Timestamp_Test Timestamp_test.cpp)
add_executable(Buffer_Test Buffer_test.cpp)

add_test(NAME fileutil_test COMMAND FileUtil_Test)
add_test(NAME timestamp_test COMMAND Timestamp_Test)
add_test(NAME buffer_test COMMAND Buffer_Test)