add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9045 )
add_subdirectory(tests)
set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
    qthelpconfig.cpp
)

set(kdevqthelp_config_UI
    qthelpconfig.ui
)

kde4_add_ui_files(kdevqthelp_SRCS ${kdevqthelp_config_UI})

kde4_add_plugin(kdevqthelp ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp
    ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevqthelp.desktop kdevqthelp_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
