include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}/project)
set(checkconfig_SRCS checkconfigwidget.cpp)
kde4_add_kcfg_files(checkconfig_SRCS checkconfig.kcfgc)
kde4_add_ui_files(checkconfig_SRCS checkconfigwidget.ui)
kde4_add_plugin(kcm_kdevcheck_config ${checkconfig_SRCS} )
target_link_libraries(kcm_kdevcheck_config
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})
install(
    TARGETS kcm_kdevcheck_config
    DESTINATION ${PLUGIN_INSTALL_DIR} )
install(
    FILES kcm_kdevcheck_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )
