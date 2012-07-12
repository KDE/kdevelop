set(filetemplate_DIRS
    cpp/cpp_basic
    cpp/private_pointer
    cpp/qt_widget
    cpp/qobject
    python/python_basic

    testing/cpp_qtestlib
    testing/python_pyunit
)

kdevplatform_add_file_templates(${filetemplate_DIRS})

install ( FILES
    common/method_declaration_cpp.txt
    common/method_definition_cpp.txt
    common/class_declaration_cpp.txt
    common/cpp_header.h
    common/cpp_implementation.cpp
    DESTINATION ${DATA_INSTALL_DIR}/kdevcodegen/templates
)
