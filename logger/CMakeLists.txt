aux_source_directory(. DIR_LIB_SRCS)

add_library(logger STATIC ${DIR_LIB_SRCS})

include_directories(${PROJECT_SOURCE_DIR}/utils)