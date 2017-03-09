add_definitions(-DTRANSLATION_DOMAIN=\"kdevheaptrack\")

set(kdevheaptrack_SRCS
    debug.cpp
    job.cpp
    plugin.cpp
    utils.cpp
    visualizer.cpp

    config/globalconfigpage.cpp
)
ki18n_wrap_ui(kdevheaptrack_UI_SRCS
    config/globalconfigpage.ui
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
    SOURCES ${kdevheaptrack_SRCS} ${kdevheaptrack_UI_SRCS}
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
install(FILES kdevheaptrack.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevheaptrack)
