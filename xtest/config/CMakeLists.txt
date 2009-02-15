
add_subdirectory(tests)

set(veritasconfig_SRCS
    configmodule.cpp configwidget.cpp)
qt4_automoc(${veritasconfig_SRCS})
kde4_add_kcfg_files(veritasconfig_SRCS veritasconfig.kcfgc)
kde4_add_ui_files(veritasconfig_SRCS veritasconfig.ui)
kde4_add_plugin(kcm_kdev_veritassettings ${veritasconfig_SRCS} )
target_link_libraries(kcm_kdev_veritassettings
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})
install(
    TARGETS kcm_kdev_veritassettings
    DESTINATION ${PLUGIN_INSTALL_DIR} )
install(
    FILES kcm_kdev_veritassettings.desktop
    DESTINATION ${SERVICES_INSTALL_DIR} )
