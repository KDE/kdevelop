add_definitions(-DTRANSLATION_DOMAIN=\"kdevheaptrack\")

find_package(heaptrack QUIET)
set_package_properties(heaptrack PROPERTIES
    DESCRIPTION "A heap memory profiler for Linux"
    URL "https://phabricator.kde.org/dashboard/view/28"
    PURPOSE "Recommended: required by the non-essential heaptrack plugin"
    TYPE RUNTIME
)
find_package(heaptrack_gui QUIET)
set_package_properties(heaptrack_gui PROPERTIES
    DESCRIPTION "Analyzer gui for heaptrack, a heap memory profiler for Linux"
    URL "https://phabricator.kde.org/dashboard/view/28"
    PURPOSE "Recommended: required by the non-essential heaptrack plugin"
    TYPE RUNTIME
)

set(kdevheaptrack_SRCS
    job.cpp
    plugin.cpp
    utils.cpp
    visualizer.cpp

    config/globalconfigpage.cpp
)
declare_qt_logging_category(kdevheaptrack_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_HEAPTRACK
    CATEGORY_BASENAME "heaptrack"
)
ki18n_wrap_ui(kdevheaptrack_SRCS
    config/globalconfigpage.ui
)
qt_add_resources(kdevheaptrack_SRCS
    kdevheaptrack.qrc
)
kconfig_add_kcfg_files(kdevheaptrack_CONFIG_SRCS
    config/globalsettings.kcfgc
)
add_library(kdevheaptrack_config STATIC
    ${kdevheaptrack_CONFIG_SRCS}
)
target_link_libraries(kdevheaptrack_config
    KDev::Shell
)
kdevplatform_add_plugin(kdevheaptrack
    SOURCES ${kdevheaptrack_SRCS}
)
target_link_libraries(kdevheaptrack
    KF6::KIOWidgets
    kdevheaptrack_config
    KDev::Project
    KDev::IExecute
)
if(KSysGuard_FOUND)
    target_link_libraries(kdevheaptrack
        kdevdebuggercommon
        KSysGuard::ProcessCore
    )
endif()
