
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE3_INCLUDE_DIRS} )


########### next target ###############

set(kdevkonsoleview_PART_SRCS
    kdevkonsoleview_part.cpp
    kdevkonsoleview.cpp
)

kde4_automoc(${kdevkonsoleview_PART_SRCS})
kde4_add_plugin(kdevkonsoleview ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview ${KDE4_KDECORE_LIBS} kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevkonsoleview)
install_targets(${PLUGIN_INSTALL_DIR} kdevkonsoleview)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevkonsoleview.desktop)

