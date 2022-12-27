include_directories(${ESYNET_SOURCE_DIR})

add_executable(FileUtil_Test FileUtil_test.cpp)

add_test(NAME fileutil_test COMMAND FileUtil_Test)