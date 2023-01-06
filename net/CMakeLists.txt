aux_source_directory(. NET_SRCS)
aux_source_directory(./poller NET_SRCS)
aux_source_directory(./timer NET_SRCS)
aux_source_directory(./thread NET_SRCS)

message(STATUS "NET_SRCS = ${NET_SRCS}")

include_directories(${ESYNET_SOURCE_DIR})

add_library(net STATIC ${NET_SRCS})
target_link_libraries(net logger fmt::fmt)