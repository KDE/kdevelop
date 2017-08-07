add_definitions(-DTRANSLATION_DOMAIN=\"kdevcppcheck\")

find_package(Cppcheck QUIET)
set_package_properties(Cppcheck PROPERTIES
    DESCRIPTION "A tool for static C/C++ code analysis"
    URL "http://cppcheck.sourceforge.net/"
    PURPOSE "Recommended: required by the non-essential Cppcheck plugin"
    TYPE RUNTIME
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(kdevcppcheck_SRCS
    plugin.cpp

    config/globalconfigpage.cpp
    config/projectconfigpage.cpp
)
ki18n_wrap_ui(kdevcppcheck_UI_SRCS
    config/globalconfigpage.ui
    config/projectconfigpage.ui
)
kconfig_add_kcfg_files(kdevcppcheck_CONFIG_SRCS
    config/globalsettings.kcfgc
    config/projectsettings.kcfgc
)
add_library(kdevcppcheck_core STATIC
    debug.cpp
    parser.cpp
    job.cpp
    parameters.cpp

    ${kdevcppcheck_CONFIG_SRCS}
)
target_link_libraries(kdevcppcheck_core
    KDev::Language
    KDev::Project
    KDev::Shell
)
kdevplatform_add_plugin(kdevcppcheck
    JSON kdevcppcheck.json
    SOURCES ${kdevcppcheck_SRCS} ${kdevcppcheck_UI_SRCS}
)
target_link_libraries(kdevcppcheck
    kdevcppcheck_core
    KF5::ItemViews
)
install(FILES kdevcppcheck.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcppcheck)

ecm_install_icons(ICONS icons/128-apps-cppcheck.png
                  DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
