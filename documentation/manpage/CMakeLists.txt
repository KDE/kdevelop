
set(kdevmanpage_SRCS
    manpagedocumentation.cpp
    manpageplugin.cpp
    manpagemodel.cpp
    manpagedocumentationwidget.cpp
)

kde4_add_plugin(kdevmanpage ${kdevmanpage_SRCS})
target_link_libraries(kdevmanpage ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${QT_QTWEBKIT_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS kdevmanpage DESTINATION ${PLUGIN_INSTALL_DIR})
configure_file(kdevmanpage.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevmanpage.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevmanpage.desktop DESTINATION ${SERVICES_INSTALL_DIR})
