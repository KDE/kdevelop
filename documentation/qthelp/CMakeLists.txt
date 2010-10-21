add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9045 )

########## shared settings ##########
set(kdevqthelp_settings_SRCS
)

kde4_add_kcfg_files(kdevqthelp_settings_SRCS qthelpsettings.kcfgc)

install(FILES qthelpsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})

set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
    ${kdevqthelp_settings_SRCS}
)

kde4_add_plugin(kdevqthelp ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp
    ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KUTILS_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevqthelp.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kdevqthelp_config_SRCS
    qthelpconfig.cpp
    ${kdevqthelp_settings_SRCS}
)

set(kdevqthelp_config_UI
    qthelpconfig.ui
)


kde4_add_ui_files(kdevqthelp_config_SRCS ${kdevqthelp_config_UI})

kde4_add_plugin( kdevqthelp_config ${kdevqthelp_config_SRCS} )

target_link_libraries( kdevqthelp_config ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS} )

install( TARGETS kdevqthelp_config DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdevqthelp_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
)

kde4_add_plugin(kdevqthelp ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
                      ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevqthelp.desktop DESTINATION ${SERVICES_INSTALL_DIR})
