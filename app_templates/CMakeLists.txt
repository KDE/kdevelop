set(apptemplate_DIRS
    cpp/CMake/cmake_qt4core
    cpp/CMake/cmake_qt4gui
    cpp/CMake/cmake_qt4core_qtest
    cpp/CMake/cmake_plaincpp
)

kdevplatform_add_app_templates(${apptemplate_DIRS})
