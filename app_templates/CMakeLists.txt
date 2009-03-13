set(apptemplate_DIRS
    cpp/CMake/cmake_qt4core
    cpp/CMake/cmake_qt4core_qtest
    cpp/QMake/qmake_qt4guiapp
    cpp/CMake/cmake_plaincpp
)

kdevplatform_add_app_templates(${apptemplate_DIRS})
