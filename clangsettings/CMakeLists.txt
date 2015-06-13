
add_library(settingsmanager
STATIC
        clangsettingsmanager.cpp
)

target_link_libraries(settingsmanager
LINK_PUBLIC
        KDev::Project
        KDev::Util
)

set(clangsettings_SRCS
        clangconfigpage.cpp
        clangsettingsplugin.cpp
        configwidget.cpp
        pathsmodel.cpp
        sessionsettings/sessionsettings.cpp
)

ki18n_wrap_ui(clangsettings_SRCS
    configwidget.ui
    sessionsettings/sessionsettings.ui
)

kconfig_add_kcfg_files( clangsettings_SRCS clangprojectconfig.kcfgc sessionsettings/sessionconfig.kcfgc)

kdevplatform_add_plugin(kdevclangsettings JSON kdevclangsettings.json SOURCES ${clangsettings_SRCS})

target_link_libraries(kdevclangsettings
LINK_PUBLIC
        KDev::Project
        KDev::Util
LINK_PRIVATE
        settingsmanager
)
