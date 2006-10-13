
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
    )


########### next target ###############

set(kdevcodeview_PART_SRCS
    kdevcodeview.cpp
    kdevcodetree.cpp
    kdevcodeview_part.cpp
)

kde4_automoc(kdevcodeview ${kdevcodeview_PART_SRCS})
kde4_add_plugin(kdevcodeview ${kdevcodeview_PART_SRCS})
target_link_libraries(kdevcodeview ${KDE4_KDECORE_LIBS} kdevplatform kdevutil ${KDE4_KHTML_LIBS})
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcodeview)
install(TARGETS kdevcodeview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcodeview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcodeview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcodeview )


