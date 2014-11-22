
include_directories(
    ${KDevPlatform_SOURCE_DIR}
)

########### Background Parser

set( bgsettings_cfg_SRCS
    bgpreferences.cpp
)

set( bgsettings_cfg_UI
    bgsettings.ui
)

ki18n_wrap_ui( bgsettings_cfg_SRCS ${bgsettings_cfg_UI} )
kconfig_add_kcfg_files( bgsettings_cfg_SRCS bgconfig.kcfgc )
add_library( kcm_kdev_bgsettings MODULE ${bgsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_bgsettings KF5::KCMUtils KDev::Shell KDev::Language)

########## Install TARGETS/Files

install( TARGETS
        kcm_kdev_bgsettings
        DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)
configure_file(kcm_kdev_bgsettings.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_bgsettings.desktop)
kcoreaddons_desktop_to_json(kcm_kdev_bgsettings ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_bgsettings.desktop)

install( FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdev_bgsettings.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} )
