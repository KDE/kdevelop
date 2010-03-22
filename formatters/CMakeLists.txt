add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9000 )

include_directories(lib)

########### astyle ###############
set(kdevastyle_PART_SRCS
    astyle_plugin.cpp
    astyle_preferences.cpp
    astyle_formatter.cpp
    lib/ASBeautifier.cpp
    lib/ASEnhancer.cpp
    lib/ASFormatter.cpp
    lib/ASResource.cpp
    lib/astyle_stringiterator.cpp
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

########### indent ###############
set(kdevindent_PART_SRCS
    indent_plugin.cpp
)

# set(indent_preferences_UI
#     indent_preferences.ui
# )

# kde4_add_ui_files(kdevastyle_PART_SRCS ${astyle_preferences_UI} )
kde4_add_plugin(kdevindent ${kdevindent_PART_SRCS})
target_link_libraries(kdevindent
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES})

install(TARGETS kdevindent DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kdevastyle.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevindent.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


add_subdirectory(tests)