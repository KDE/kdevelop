add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9000 )

########### next target ###############
set(kdevindent_PART_SRCS
    indent_plugin.cpp
)

# set(kdevindent_UI
#     kdevindent_preferences.ui
# )

# kde4_add_ui_files(kdevindent_PART_SRCS ${astyle_preferences_UI} )
kde4_add_plugin(kdevindent ${kdevindent_PART_SRCS})
target_link_libraries(kdevindent
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY})

install(TARGETS kdevindent DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kdevindent.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
