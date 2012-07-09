
set(kdevfiletemplates_PART_SRCS
    templatepreview.cpp
    filetemplatesplugin.cpp
)

kde4_add_plugin(kdevfiletemplates ${kdevfiletemplates_PART_SRCS})
target_link_libraries(kdevfiletemplates
    ${KDE4_KDECORE_LIBS}
    kdevplatforminterfaces
    kdevplatformlanguage)

install(TARGETS kdevfiletemplates DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevfiletemplates.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

