# Runtime - Standalone Vulkan test application
# Enable with: -DBUILD_RUNTIME=ON

find_package(glfw3 QUIET)

if(NOT glfw3_FOUND)
    message(WARNING "GLFW not found - runtime target will not be available. "
                    "Install with: sudo apt install libglfw3-dev")
    return()
endif()

set(TARGET_NAME runtime)

set(SOURCES
        main.cpp
        runtime_app.cpp
        glfw_platform_config.cpp
        glfw_input_translator.cpp
)

set(HEADERS
        runtime_app.h
        glfw_initializer.h
        glfw_platform_config.h
        glfw_input_translator.h
)

add_executable(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        glfw
        engine
)

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

file(GLOB RUNTIME_SHADER_FILES
        ${CMAKE_SOURCE_DIR}/src/engine/shaders/*.slang)
if(RUNTIME_SHADER_FILES)
    add_slang_shader_target(runtime_shaders
        SOURCES ${RUNTIME_SHADER_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
    )
    add_dependencies(${TARGET_NAME} runtime_shaders)
endif()

file(GLOB RUNTIME_TEXTURE_FILES
        ${CMAKE_SOURCE_DIR}/src/engine/textures/*.jpeg
        ${CMAKE_SOURCE_DIR}/src/engine/textures/*.png
        ${CMAKE_SOURCE_DIR}/src/engine/textures/*.jpg)
if(RUNTIME_TEXTURE_FILES)
    file(COPY ${RUNTIME_TEXTURE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/mv_spartan)
    file(COPY ${CMAKE_SOURCE_DIR}/mv_spartan
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)
endif()

message(STATUS "runtime standalone app target enabled")
