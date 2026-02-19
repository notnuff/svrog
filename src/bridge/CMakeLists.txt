set(TARGET_NAME bridge)

set(SOURCES vulkandemorenderer.cpp)
set(HEADERS ivulkanrenderer.h vulkandemorenderer.h)

add_library(${TARGET_NAME}
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
        Vulkan::Vulkan
)

target_include_directories(${TARGET_NAME}
        PUBLIC
#        well, that's bullshit, need to use proper library include/headers.h
        ./../

)