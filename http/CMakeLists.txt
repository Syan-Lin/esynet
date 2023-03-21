aux_source_directory(. DIR_LIB_SRCS)

message(STATUS "HTTP_SRCS = ${DIR_LIB_SRCS}")

include_directories(${ESYNET_SOURCE_DIR})

add_library(http STATIC ${DIR_LIB_SRCS})