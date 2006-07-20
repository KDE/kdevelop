
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
)

### Configuration module for general kdevelop settings

set( kdevenv_config_SRC
kdevenvpreferences.cpp
kdevenvwidget.cpp
)
set( kdevenv_config_UI
kdevenvsettings.ui
)
kde4_automoc( ${kdevenv_config_SRC} )
kde4_add_ui_files( kdevenv_config_SRC ${kdevenv_config_UI} )
kde4_add_plugin(kcm_kdevenv_settings ${kdevenv_config_SRC})
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kdevenv_settings )
target_link_libraries(kcm_kdevenv_settings ${KDE4_KUTILS_LIBS} kdevinterfaces )
install(TARGETS kcm_kdevenv_settings  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kcm_kdevenv_settings.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
