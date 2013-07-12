
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9524 )


########### next target ###############

set(kdevcontextbrowser_PART_SRCS
    contextbrowser.cpp
    contextbrowserview.cpp
    browsemanager.cpp
)

kde4_add_plugin(kdevcontextbrowser ${kdevcontextbrowser_PART_SRCS})
target_link_libraries(kdevcontextbrowser kdevplatforminterfaces kdevplatformutil kdevplatformlanguage ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kdevcontextbrowser DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevcontextbrowser.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcontextbrowser)

########### install files ###############

configure_file(kdevcontextbrowser.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcontextbrowser.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcontextbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
