set(TARGET_NAME logging)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(${TARGET_NAME} STATIC
    colored_logger.cpp
    colored_logger.h
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(${TARGET_NAME} PUBLIC
    Qt6::Core
)

set_target_properties(${TARGET_NAME} PROPERTIES
    AUTOMOC ON
)

