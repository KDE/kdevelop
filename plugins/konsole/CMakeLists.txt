kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE3_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevkonsoleview_PART_SRCS
    konsoleviewpart.cpp
    konsoleviewwidget.cpp
)

kde4_automoc(${kdevkonsoleview_PART_SRCS})
kde4_add_plugin(kdevkonsoleview ${kdevkonsoleview_PART_SRCS})
target_link_libraries(kdevkonsoleview ${KDE4_KDECORE_LIBS} kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevinterfaces)
install_targets(${PLUGIN_INSTALL_DIR} kdevkonsoleview)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevkonsoleview.desktop)

kde4_footer()
