########### indent ###############
set(kdevindent_PART_SRCS
    indent_plugin.cpp
)

kde4_add_plugin(kdevindent ${kdevindent_PART_SRCS})
target_link_libraries(kdevindent
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES})

install(TARGETS kdevindent DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kdevindent.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
