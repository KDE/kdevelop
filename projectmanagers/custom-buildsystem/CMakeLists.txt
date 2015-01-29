project(kdevelop4-custom-buildsystem)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustombuildsystem\")
## Unittests
add_subdirectory( tests )

## KDevelop Plugin
set(custom_SRCS
    custombuildsystemconfig.cpp
    custombuildsystemplugin.cpp
    custombuildjob.cpp
    configwidget.cpp
    custombuildsystemconfigwidget.cpp
    kcm_custombuildsystem.cpp
    debugarea.cpp
)
ki18n_wrap_ui(custom_SRCS configwidget.ui custombuildsystemconfigwidget.ui)
kconfig_add_kcfg_files(custom_SRCS kcfg_custombuildsystemconfig.kcfgc )

kdevplatform_add_plugin(kdevcustombuildsystem JSON kdevcustombuildsystem.json SOURCES ${custom_SRCS})
target_link_libraries(kdevcustombuildsystem
      KDev::Project
      KDev::Interfaces
      KDev::Util
      KDev::OutputView
)
