project(kdevelop4-custom-buildsystem)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustombuildsystem\")
## Unittests
add_subdirectory( tests )

## KDevelop Plugin
set(custom_SRCS
    custombuildsystemplugin.cpp
    custombuildjob.cpp
    configwidget.cpp
    custombuildsystemconfigwidget.cpp
    kcm_custombuildsystem.cpp
    debugarea.cpp
)
ki18n_wrap_ui(custom_SRCS configwidget.ui custombuildsystemconfigwidget.ui)
kconfig_add_kcfg_files(custom_SRCS kcfg_custombuildsystemconfig.kcfgc )
add_library(kdevcustombuildsystem MODULE ${custom_SRCS})
target_link_libraries(kdevcustombuildsystem
      KDev::Project
      KDev::Interfaces
      KDev::Util
      KDev::OutputView
)

## Installation
install(TARGETS kdevcustombuildsystem DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

configure_file(kdevcustombuildsystem.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop)
kcoreaddons_desktop_to_json(kdevcustombuildsystem ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcustombuildsystem.desktop DESTINATION ${SERVICES_INSTALL_DIR})
