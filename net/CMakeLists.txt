aux_source_directory(. DIR_LIB_SRCS)
aux_source_directory(./poller DIR_LIB_SRCS)

message(STATUS "DIR_LIB_SRCS = ${DIR_LIB_SRCS}")

include_directories(${ESYNET_SOURCE_DIR})

add_library(net STATIC ${DIR_LIB_SRCS})
target_link_libraries(net logger fmt::fmt)