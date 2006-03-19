kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevmakebuilder_LIB_SRCS
    kdevmakebuilder.cpp
)

kde4_automoc(${kdevmakebuilder_LIB_SRCS})
kde4_add_plugin(kdevmakebuilder ${kdevmakebuilder_LIB_SRCS})
target_link_libraries(kdevmakebuilder ${QT_AND_KDECORE_LIBS} kdevelop)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevmakebuilder)
install_targets(${PLUGIN_INSTALL_DIR} kdevmakebuilder)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevmakebuilder.desktop)


kde4_footer()
