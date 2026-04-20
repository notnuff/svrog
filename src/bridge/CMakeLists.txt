set(TARGET_NAME bridge)

set(SOURCES render_engine.cpp)
set(HEADERS render_engine.h qml_initializer.h)

add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
        Qt6::Quick
        Vulkan::Vulkan
        renderer
)

target_include_directories(${TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)