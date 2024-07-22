set(filetemplate_DIRS
    classes/cpp_basic
    classes/private_pointer
    classes/python_basic
    classes/c_gobject
    classes/c_gobject_private
    classes/c_gobject_properties
    classes/qt_shared
    classes/qt_interface
    classes/qobject
    classes/qobject_pimpl
    classes/qabstractitemmodel
    classes/qabstractitemmodel_pimpl
    classes/qt_widget
    classes/qwidget_pimpl
    classes/qdialog
    classes/qdialog_pimpl

    testing/cpp_cpputest
    testing/cpp_gtest
    testing/cpp_qtestlib
    testing/cpp_qtestlib_kdevelop
    testing/python_pyunit
    testing/php_phpunit

    other/cmake_module
    other/qt_qrc_file
)

kdevplatform_add_file_templates(${filetemplate_DIRS})

install ( FILES
    common/method_declaration_cpp.txt
    common/method_definition_cpp.txt
    common/license_header_cpp.txt
    common/class_declaration_cpp.txt
    common/class_declaration_apidox_cpp.txt
    common/class_method_declaration_cpp.txt
    common/class_method_declaration_apidox_cpp.txt
    common/class_property_setter_declaration_cpp.txt
    common/class_property_setter_declaration_apidox_cpp.txt
    common/class_property_setter_definition_cpp.txt
    common/class_property_getter_declaration_cpp.txt
    common/class_property_getter_declaration_apidox_cpp.txt
    common/class_property_getter_definition_cpp.txt
    common/class_property_signal_declaration_cpp.txt
    common/class_qproperty_declaration_cpp.txt
    common/cpp_header.h
    common/cpp_header_onlyfunctions.h
    common/cpp_implementation.cpp
    common/cpp_qobject_header.h
    common/cpp_qobject_implementation.cpp
    common/cpp_qobject_pimpl_header.h
    common/cpp_qobject_pimpl_p_header.h
    common/cpp_qobject_pimpl_implementation.cpp
    DESTINATION ${KDE_INSTALL_DATADIR}/kdevcodegen/templates
)
