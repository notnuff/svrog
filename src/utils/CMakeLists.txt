set(TARGET_NAME utils)

set(SOURCES
    file_utils.cpp
)

set(HEADERS
    file_utils.h
)

add_library(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${TARGET_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

