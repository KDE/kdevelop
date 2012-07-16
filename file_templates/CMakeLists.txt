set(filetemplate_DIRS
    classes/cpp_basic
    classes/private_pointer
    classes/qt_widget
    classes/qobject
    classes/python_basic
    classes/c_gobject
    classes/c_gobject_private
    classes/c_gobject_properties
    
    testing/cpp_qtestlib
    testing/cpp_qtestlib_kde
    testing/cpp_qtestlib_kdevelop
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
