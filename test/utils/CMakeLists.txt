include_directories(${PROJECT_SOURCE_DIR}/utils)

add_executable(FileUtil_Test FileUtil_test.cpp)

add_test(NAME fileutil_test COMMAND FileUtil_Test)