set(TARGET_NAME renderer)

set(SOURCES instance.cpp)
set(HEADERS instance.h)

include_directories(. PUBLIC)
add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        Vulkan::Vulkan
)

target_include_directories(${TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)