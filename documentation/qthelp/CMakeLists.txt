
set(kdevqthelp_SRCS
    qthelpplugin.cpp
)

kde4_add_plugin(kdevqthelp ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
                      kdevplatformlanguage kdevplatforminterfaces)

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevqthelp.desktop DESTINATION ${SERVICES_INSTALL_DIR})
