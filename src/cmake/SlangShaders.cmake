function (add_slang_shader_target TARGET)
    cmake_parse_arguments ("SHADER" "" "OUTPUT_DIR" "SOURCES" ${ARGN})

    if(NOT SLANGC_EXECUTABLE)
        message(WARNING "slangc not found - skipping shader compilation for target ${TARGET}")
        return()
    endif()

    if(NOT SHADER_SOURCES)
        message(FATAL_ERROR "add_slang_shader_target: No SOURCES specified for target ${TARGET}")
    endif()

    if(NOT SHADER_OUTPUT_DIR)
        set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    endif()

    set (ENTRY_POINTS -entry vertMain -entry fragMain)

    add_custom_command (
            OUTPUT ${SHADER_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
    )

    add_custom_command (
            OUTPUT  ${SHADER_OUTPUT_DIR}/slang.spv
            COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES}
                -target spirv
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name
                ${ENTRY_POINTS}
                -o slang.spv
            WORKING_DIRECTORY ${SHADER_OUTPUT_DIR}
            DEPENDS ${SHADER_OUTPUT_DIR} ${SHADER_SOURCES}
            COMMENT "Compiling Slang shaders for target ${TARGET}"
            VERBATIM
    )

    add_custom_target (${TARGET} ALL DEPENDS ${SHADER_OUTPUT_DIR}/slang.spv)
endfunction()