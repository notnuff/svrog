function(collect_sources_recursive DIR SOURCES_VAR HEADERS_VAR)
    file(GLOB LOCAL_SOURCES "${DIR}/*.cpp")
    file(GLOB LOCAL_HEADERS "${DIR}/*.h")

    list(APPEND ${SOURCES_VAR} ${LOCAL_SOURCES})
    list(APPEND ${HEADERS_VAR} ${LOCAL_HEADERS})

    file(GLOB SUBDIRS RELATIVE ${DIR} ${DIR}/*)

    foreach(SUBDIR ${SUBDIRS})
        set(FULL_SUBDIR "${DIR}/${SUBDIR}")

        if(IS_DIRECTORY ${FULL_SUBDIR})
            # Skip directories with CMakeLists.txt,
            # as their sources must be handled by respective CMakeLists.txt
            if(NOT EXISTS "${FULL_SUBDIR}/CMakeLists.txt")
                collect_sources_recursive(${FULL_SUBDIR} ${SOURCES_VAR} ${HEADERS_VAR})
            endif()
        endif()
    endforeach()

    set(${SOURCES_VAR} ${${SOURCES_VAR}} PARENT_SCOPE)
    set(${HEADERS_VAR} ${${HEADERS_VAR}} PARENT_SCOPE)
endfunction()
