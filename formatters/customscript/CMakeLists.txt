########### indent ###############
set(kdevcustomscript_PART_SRCS
    customscript_plugin.cpp
)

kde4_add_plugin(kdevcustomscript ${kdevcustomscript_PART_SRCS})
target_link_libraries(kdevcustomscript
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES})

install(TARGETS kdevcustomscript DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kdevcustomscript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
