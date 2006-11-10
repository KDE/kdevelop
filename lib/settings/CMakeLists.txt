
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
)

### Configuration module for general kdevelop settings

set( kdev_config_SRC
    kdevprojectpreferences.cpp
    kdevuipreferences.cpp
    kdevenvpreferences.cpp
    kdevenvwidget.cpp
    kdevbgpreferences.cpp
#     kdevpluginpreferences.cpp
)
set( kdevelop_UI
    projectsettings.ui
    uisettings.ui
    envsettings.ui
    bgsettings.ui
)
set( kdevelop_KCFGC
    kdevprojectconfig.kcfgc
    kdevbgconfig.kcfgc
    kdevuiconfig.kcfgc
)
set( kdevelop_DESKTOP
    kcm_kdevproject_settings.desktop
    kcm_kdevui_settings.desktop
    kcm_kdevenv_settings.desktop
    kcm_kdevbg_settings.desktop
#     kcm_kdevplugin_settings.desktop
)

kde4_add_ui_files( kdev_config_SRC ${kdevelop_UI} )
kde4_automoc(${kdev_config_SRC})

kde4_add_kcfg_files(kdev_config_SRC ${kdevelop_KCFGC} )

kde4_add_plugin(kcm_kdevelop_settings ${kdev_config_SRC})

target_link_libraries(kcm_kdevelop_settings ${KDE4_KUTILS_LIBS} kdevplatform )
install(TARGETS kcm_kdevelop_settings  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES ${kdevelop_DESKTOP} DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES data.kdev4 kdevelop.setdlg DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
