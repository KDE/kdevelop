add_definitions(-DTRANSLATION_DOMAIN=\"kdevshellcheck\")

#find_package(???? QUIET)
#set_package_properties(shellcheck PROPERTIES
#    DESCRIPTION "A static analysis and linting tool for sh/bash scripts"
#    URL "https://www.shellcheck.net/"
#    PURPOSE "Recommended: required by the non-essential shellcheck plugin"
#    TYPE RUNTIME
#)


ecm_qt_declare_logging_category(kdevshellcheck_core_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_SHELLCHECK
    CATEGORY_NAME "kdevelop.plugins.shellcheck"
)

kconfig_add_kcfg_files(kdevshellcheck_core_SRCS
    config/globalsettings.kcfgc
)

add_library(kdevshellcheck_core STATIC
    ${kdevshellcheck_core_SRCS}
)

target_link_libraries(kdevshellcheck_core
    KDev::Interfaces
)

set(kdevshellcheck_SRCS
    shellcheck.cpp

    config/globalconfigpage.cpp
)

ki18n_wrap_ui(kdevshellcheck_SRCS
    config/globalconfigpage.ui
)

qt5_add_resources(kdevshellcheck_SRCS
    kdevshellcheck.qrc
)

kdevplatform_add_plugin(kdevshellcheck
    JSON kdevshellcheck.json
    SOURCES ${kdevshellcheck_SRCS}
)

target_link_libraries(kdevshellcheck
    kdevshellcheck_core
)

ecm_install_icons(ICONS icons/128-apps-shellcheck.png
                  DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

