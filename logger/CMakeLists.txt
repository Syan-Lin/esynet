aux_source_directory(. DIR_LIB_SRCS)

include_directories(${ESYNET_SOURCE_DIR})

add_library(logger STATIC ${DIR_LIB_SRCS})