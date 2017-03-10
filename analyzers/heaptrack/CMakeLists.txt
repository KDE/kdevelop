add_definitions(-DTRANSLATION_DOMAIN=\"kdevheaptrack\")

set(kdevheaptrack_SRCS
    debug.cpp
    job.cpp
    plugin.cpp
    utils.cpp
    visualizer.cpp

    config/globalconfigpage.cpp
)
ki18n_wrap_ui(kdevheaptrack_SRCS
    config/globalconfigpage.ui
)
qt5_add_resources(kdevheaptrack_SRCS
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
    JSON kdevheaptrack.json
    SOURCES ${kdevheaptrack_SRCS}
)
target_link_libraries(kdevheaptrack
    kdevheaptrack_config
    KDev::Project
)
if(KF5SysGuard_FOUND)
    target_link_libraries(kdevheaptrack
        kdevdebuggercommon
        KF5::ProcessUi
    )
endif()
