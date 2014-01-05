
set(kdevtemplatemanager_config_SRCS
    templateconfig.cpp
    templatepage.cpp 
)

qt5_wrap_ui(kdevtemplatemanager_config_SRCS templateconfig.ui templatepage.ui)

add_library( kdevtemplatemanager_config MODULE ${kdevtemplatemanager_config_SRCS} )

target_link_libraries( kdevtemplatemanager_config ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF3_LIBS} kdevplatforminterfaces)

install( TARGETS kdevtemplatemanager_config DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdevtemplatemanager_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

