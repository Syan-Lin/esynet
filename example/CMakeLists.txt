include_directories(${ESYNET_SOURCE_DIR})

add_executable(use_log use_log.cpp)
target_link_libraries(use_log fmt::fmt logger)