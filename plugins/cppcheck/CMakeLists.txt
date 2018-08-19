add_definitions(-DTRANSLATION_DOMAIN=\"kdevcppcheck\")

add_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

find_package(Cppcheck QUIET)
set_package_properties(Cppcheck PROPERTIES
    DESCRIPTION "A tool for static C/C++ code analysis"
    URL "http://cppcheck.sourceforge.net/"
    PURPOSE "Recommended: required by the non-essential Cppcheck plugin"
    TYPE RUNTIME
)

set(kdevcppcheck_core_SRCS
    parser.cpp
    job.cpp
    parameters.cpp
    utils.cpp
)
ecm_qt_declare_logging_category(kdevcppcheck_core_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_CPPCHECK
    CATEGORY_NAME "kdevelop.plugins.cppcheck"
)
kconfig_add_kcfg_files(kdevcppcheck_core_SRCS
    config/globalsettings.kcfgc
    config/projectsettings.kcfgc
)
add_library(kdevcppcheck_core STATIC
    ${kdevcppcheck_core_SRCS}
)
target_link_libraries(kdevcppcheck_core
    KDev::Language
    KDev::Project
    KDev::Shell
)

set(kdevcppcheck_SRCS
    plugin.cpp
    problemmodel.cpp

    config/globalconfigpage.cpp
    config/projectconfigpage.cpp
)
ki18n_wrap_ui(kdevcppcheck_SRCS
    config/globalconfigpage.ui
    config/projectconfigpage.ui
)
qt5_add_resources(kdevcppcheck_SRCS
    kdevcppcheck.qrc
)
kdevplatform_add_plugin(kdevcppcheck
    JSON kdevcppcheck.json
    SOURCES ${kdevcppcheck_SRCS}
)
target_link_libraries(kdevcppcheck
    kdevcppcheck_core
    KF5::ItemViews
)

ecm_install_icons(ICONS icons/128-apps-cppcheck.png
                  DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
