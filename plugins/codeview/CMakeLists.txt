kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
    ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevcodeview_PART_SRCS
    kdevcodeview.cpp
    kdevcodeview_part.cpp
)

kde4_automoc(${kdevcodeview_PART_SRCS})
kde4_add_plugin(kdevcodeview ${kdevcodeview_PART_SRCS})
target_link_libraries(kdevcodeview ${KDE4_KDECORE_LIBS} kdevinterfaces kdevutil khtml)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcodeview)
install_targets(${PLUGIN_INSTALL_DIR} kdevcodeview)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevcodeview.desktop)
install_files(${DATA_INSTALL_DIR}/kdevcodeview FILES kdevcodeview.rc)


kde4_footer()
