
include_directories(
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}
)


########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleview_part.cpp
    kdevkonsoleview.cpp
)

kde4_add_plugin(kdevkonsoleview ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview ${KDE4_KDECORE_LIBS} kdevplatforminterfaces)

install(TARGETS kdevkonsoleview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevkonsoleview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

