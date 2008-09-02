
add_subdirectory(tests)

set(veritasconfig_SRCS
    configmodule.cpp configwidget.cpp)
kde4_add_kcfg_files(veritasconfig_SRCS veritasconfig.kcfgc)
kde4_add_ui_files(veritasconfig_SRCS veritasconfig.ui)
kde4_add_plugin(kcm_kdevveritas_config ${veritasconfig_SRCS} )
target_link_libraries(kcm_kdevveritas_config
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})
install(
    TARGETS kcm_kdevveritas_config
    DESTINATION ${PLUGIN_INSTALL_DIR} )
install(
    FILES kcm_kdevveritas_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )
