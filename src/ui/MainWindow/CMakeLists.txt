set(MODULE_NAME MainWindow)
set(TARGET_NAME ${MODULE_NAME}Lib)

qt_add_library(${TARGET_NAME} STATIC)
set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC ON)
target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Quick)

qt_add_qml_module(${TARGET_NAME}
    URI ${MODULE_NAME}
    RESOURCE_PREFIX /svrog.com/imports
    QML_FILES
        Main.qml
)

