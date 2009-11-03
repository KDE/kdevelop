add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9045 )

set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpdocumentation.cpp
)

kde4_add_plugin(kdevqthelp ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
                      ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevqthelp.desktop DESTINATION ${SERVICES_INSTALL_DIR})
