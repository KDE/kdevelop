
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util )


########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleview_part.cpp
    kdevkonsoleview.cpp
)

kde4_automoc(kdevkonsoleview ${kdevkonsoleview_PART_SRCS})
kde4_add_plugin(kdevkonsoleview ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview ${KDE4_KDECORE_LIBS} kdevplatform)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevkonsoleview)
install(TARGETS kdevkonsoleview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevkonsoleview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

