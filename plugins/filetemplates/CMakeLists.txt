
set(kdevfiletemplates_PART_SRCS
    filetemplatesplugin.cpp
)

kde4_add_ui_files(kdevfiletemplates_PART_SRCS ${kdevfiletemplates_PART_UI})

kde4_add_plugin(kdevfiletemplates ${kdevfiletemplates_PART_SRCS})
target_link_libraries(kdevfiletemplates
    ${KDE4_KDECORE_LIBS}
    kdevplatforminterfaces
    kdevplatformlanguage)

install(TARGETS kdevfiletemplates DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevfiletemplates.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

