set(TARGET_NAME renderer)

find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(SOURCES "")
set(HEADERS "")
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR}/vk_ctx SOURCES HEADERS)
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR}/shaders SOURCES HEADERS)
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR}/utils SOURCES HEADERS)

add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PUBLIC
        Vulkan::Vulkan
        Qt6::Core
        utils
)

target_include_directories(${TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)