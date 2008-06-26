include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}/project)
set(cppunitconfig_SRCS cppunitconfigwidget.cpp)
kde4_add_kcfg_files(cppunitconfig_SRCS cppunitconfig.kcfgc)
kde4_add_ui_files(cppunitconfig_SRCS cppunitconfigwidget.ui)
kde4_add_plugin(kcm_kdevcppunit_config ${cppunitconfig_SRCS} )
target_link_libraries(kcm_kdevcppunit_config
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})
install(
    TARGETS kcm_kdevcppunit_config
    DESTINATION ${PLUGIN_INSTALL_DIR} )
install(
    FILES kcm_kdevcppunit_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )
