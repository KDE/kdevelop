add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9045 )


########## shared settings ##########
set(kdevqch_settings_SRCS
)

kde4_add_kcfg_files(kdevqch_settings_SRCS qchsettings.kcfgc)

install(FILES qchsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})

set(kdevqch_SRCS
    qthelpplugin.cpp
    qthelpdocumentation.cpp
    ${kdevqch_settings_SRCS}

)

kde4_add_plugin(kdevqch ${kdevqch_SRCS})
target_link_libraries(kdevqch
    ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KUTILS_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevqch DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevqch.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kdevqch_config_SRCS
    qchconfig.cpp
    ${kdevqch_settings_SRCS}
)

set(kdevqch_config_UI
    qchconfig.ui
)


kde4_add_ui_files(kdevqch_config_SRCS ${kdevqch_config_UI})

kde4_add_plugin( kdevqch_config ${kdevqch_config_SRCS} )

target_link_libraries( kdevqch_config ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} )

install( TARGETS kdevqch_config DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdevqch_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
