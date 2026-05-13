set(TARGET_NAME engine)

find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(glm REQUIRED)
find_package(TinyGLTF REQUIRED)

set(SOURCES "")
set(HEADERS "")
collect_sources_recursive(${CMAKE_CURRENT_SOURCE_DIR} SOURCES HEADERS)

add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PUBLIC
        Vulkan::Vulkan
        Qt6::Core
        Qt6::Gui
        glm::glm
        utils
        logging
)

target_link_libraries(${TARGET_NAME} PRIVATE
        TinyGLTF::TinyGLTF
)

target_include_directories(${TARGET_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
