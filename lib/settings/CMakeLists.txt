
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
)

### Configuration module for general kdevelop settings

set( kdev_config_SRC
    kdeveloppreferences.cpp
    kdevenvpreferences.cpp
    kdevenvwidget.cpp
)
set( kdevelop_UI
    settingswidget.ui
    kdevenvsettings.ui
)

kde4_automoc( ${kdev_config_SRC} )
kde4_add_ui_files( kdev_config_SRC ${kdevelop_UI} )
kde4_add_kcfg_files(kdev_config_SRC kdevelopconfig.kcfgc )
kde4_add_plugin(kcm_kdevelop_settings ${kdev_config_SRC})
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdevelop_settings )
target_link_libraries(kcm_kdevelop_settings ${KDE4_KUTILS_LIBS} kdevplatform )
install(TARGETS kcm_kdevelop_settings  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kcm_kdevelop_settings.desktop kcm_kdevenv_settings.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
