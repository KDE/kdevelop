add_definitions(-DTRANSLATION_DOMAIN=\"kdevclazy\")

find_package(ClazyStandalone QUIET)
set_package_properties(ClazyStandalone PROPERTIES
    DESCRIPTION "Qt oriented code checker based on clang framework. Krazy's little brother"
    URL "https://commits.kde.org/clazy"
    PURPOSE "Recommended: required by the non-essential Clazy plugin"
    TYPE RUNTIME
)

set(kdevclazy_core_SRCS
    checksdb.cpp
    job.cpp
    jobparameters.cpp
    utils.cpp
    checksetselection.cpp
    checksetselectionlock.cpp
    checksetselectionmanager.cpp
)
declare_qt_logging_category(kdevclazy_core_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_CLAZY
    CATEGORY_BASENAME "clazy"
)
kconfig_add_kcfg_files(kdevclazy_core_SRCS GENERATE_MOC
    config/globalsettings.kcfgc
)
kconfig_add_kcfg_files(kdevclazy_core_SRCS
    config/projectsettings.kcfgc
)
add_library(kdevclazy_core STATIC
    ${kdevclazy_core_SRCS}
)
target_link_libraries(kdevclazy_core
    KDevCompileAnalyzerCommon
    KDev::Project
    KDev::Shell
)

set(kdevclazy_SRCS
    analyzer.cpp
    plugin.cpp

    config/customchecksetconfigproxywidget.cpp
    config/checksetselectioncombobox.cpp
    config/checksetselectionlistmodel.cpp
    config/checksetmanagewidget.cpp
    config/checkswidget.cpp
    config/commandlinewidget.cpp
    config/globalconfigpage.cpp
    config/projectconfigpage.cpp
)
ki18n_wrap_ui(kdevclazy_SRCS
    config/checksetmanagewidget.ui
    config/checkswidget.ui
    config/commandlinewidget.ui
    config/globalconfigpage.ui
    config/projectconfigpage.ui
)
qt_add_resources(kdevclazy_SRCS
    kdevclazy.qrc
)
kdevplatform_add_plugin(kdevclazy
    SOURCES ${kdevclazy_SRCS}
)
target_link_libraries(kdevclazy
    kdevclazy_core
    KF6::ItemViews
    KF6::KIOWidgets
)

ecm_install_icons(ICONS icons/128-apps-clazy.png
                  DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
