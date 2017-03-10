add_definitions(-DTRANSLATION_DOMAIN=\"kdevcppcheck\")

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
kconfig_add_kcfg_files(kdevcppcheck_CONFIG_SRCS
    config/globalsettings.kcfgc
    config/projectsettings.kcfgc
)
add_library(kdevcppcheck_core STATIC
    debug.cpp
    parser.cpp
    job.cpp
    parameters.cpp
    utils.cpp

    ${kdevcppcheck_CONFIG_SRCS}
)
target_link_libraries(kdevcppcheck_core
    KDev::Language
    KDev::Project
    KDev::Shell
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

add_subdirectory(tests)
