include_directories(${PROJECT_SOURCE_DIR}/utils)
include_directories(${PROJECT_SOURCE_DIR}/logger)

add_executable(use_log use_log.cpp)
target_link_libraries(use_log fmt::fmt)
target_link_libraries(use_log logger)