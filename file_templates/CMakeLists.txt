set(filetemplate_DIRS
    cpp/cpp_basic
    cpp/private_pointer
    cpp/qt_widget
    cpp/qobject
    python/python_basic
)

kdevplatform_add_file_templates(${filetemplate_DIRS})

install ( FILES
    common/method_declaration_cpp.txt
    common/method_definition_cpp.txt
    DESTINATION ${DATA_INSTALL_DIR}/kdevcodegen/templates
)
