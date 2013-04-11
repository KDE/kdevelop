
set(kdevtemplatemanager_config_SRCS
    templateconfig.cpp
    templatepage.cpp 
)

kde4_add_ui_files(kdevtemplatemanager_config_SRCS templateconfig.ui templatepage.ui)

kde4_add_plugin( kdevtemplatemanager_config ${kdevtemplatemanager_config_SRCS} )

target_link_libraries( kdevtemplatemanager_config ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF3_LIBS} kdevplatforminterfaces)

install( TARGETS kdevtemplatemanager_config DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdevtemplatemanager_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

