set( uisettings_cfg_SRCS
    uipreferences.cpp
)

set( uisettings_cfg_UI
    uiconfig.ui
)

kde4_automoc(${uisettings_cfg_SRCS})
kde4_add_ui_files( uisettings_cfg_SRCS ${uisettings_cfg_UI} )
kde4_add_kcfg_files( uisettings_cfg_SRCS uiconfig.kcfgc )
kde4_add_plugin( kcm_kdev_uisettings ${uisettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_uisettings ${KDE4_KUTILS_LIBS} )

install( TARGETS kcm_kdev_uisettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_uisettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
