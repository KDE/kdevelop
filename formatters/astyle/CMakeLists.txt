include_directories(lib)

set(kdevastyle_PART_SRCS
    astyle_plugin.cpp
    astyle_preferences.cpp
    astyle_formatter.cpp
    astyle_stringiterator.cpp
    lib/ASBeautifier.cpp
    lib/ASEnhancer.cpp
    lib/ASFormatter.cpp
    lib/ASResource.cpp
)

set(astyle_preferences_UI
    astyle_preferences.ui
)

kde4_add_ui_files(kdevastyle_PART_SRCS ${astyle_preferences_UI} )
kde4_add_plugin(kdevastyle ${kdevastyle_PART_SRCS})
target_link_libraries(kdevastyle
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES})

install(TARGETS kdevastyle DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevastyle.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevastyle.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevastyle.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

add_subdirectory(tests)
