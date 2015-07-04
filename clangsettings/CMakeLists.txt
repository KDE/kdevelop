
add_library(settingsmanager
STATIC
        clangsettingsmanager.cpp
)

target_link_libraries(settingsmanager
LINK_PUBLIC
        KDev::Project
        KDev::Util
)

set(sessionsettings_SRCS
    sessionsettings/sessionsettings.cpp
)

ki18n_wrap_ui(sessionsettings_SRCS
    sessionsettings/sessionsettings.ui
)

kconfig_add_kcfg_files(sessionsettings_SRCS sessionsettings/sessionconfig.kcfgc)

add_library(sessionsettings
STATIC
        ${sessionsettings_SRCS}
)

target_link_libraries(sessionsettings
LINK_PUBLIC
        KDev::Project
        KDev::Util
)
