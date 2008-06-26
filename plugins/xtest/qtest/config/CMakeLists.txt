include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}/project)
set(qtestconfig_SRCS qtestconfigwidget.cpp)
kde4_add_kcfg_files(qtestconfig_SRCS qtestconfig.kcfgc)
kde4_add_ui_files(qtestconfig_SRCS qtestconfigwidget.ui)
kde4_add_plugin(kcm_kdevqtest_config ${qtestconfig_SRCS} )
target_link_libraries(kcm_kdevqtest_config
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})
install(
    TARGETS kcm_kdevqtest_config
    DESTINATION ${PLUGIN_INSTALL_DIR} )
install(
    FILES kcm_kdevqtest_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )
