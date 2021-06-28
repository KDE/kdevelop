add_definitions(-DTRANSLATION_DOMAIN=\"kdevsourceformatter\")

set(kdevsourceformatter_CONFIG_SRCS
    config/projectconfigpage.cpp
)

ki18n_wrap_ui(kdevsourceformatter_CONFIG_SRCS
    config/projectconfigpage.ui
)

kconfig_add_kcfg_files(kdevsourceformatter_CONFIG_SRCS
    config/projectconfig.kcfgc
)

set(kdevsourceformatter_SRCS
    sourceformatterplugin.cpp
    ${kdevsourceformatter_CONFIG_SRCS}
)

kdevplatform_add_plugin(kdevsourceformatter
    SOURCES ${kdevsourceformatter_SRCS}
)

target_link_libraries(kdevsourceformatter
    KDev::Interfaces
    KDev::Project
    KDev::Shell
)
