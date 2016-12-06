add_definitions(-DTRANSLATION_DOMAIN=\"kdevcppcheck\")

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

add_subdirectory(tests)
