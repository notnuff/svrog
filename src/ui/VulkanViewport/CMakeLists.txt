set(MODULE_NAME VulkanViewport)
set(TARGET_NAME ${MODULE_NAME}Lib)

qt_add_library(${TARGET_NAME} STATIC)
set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC ON)
target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Quick bridge)

qt_add_qml_module(${TARGET_NAME}
    URI ${MODULE_NAME}
    SOURCES
        vulkanviewport.cpp
        vulkanviewport.h
)