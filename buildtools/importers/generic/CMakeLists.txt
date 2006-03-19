kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevgenericimporter_PART_SRCS
    kdevgenericimporter.cpp
)

kde4_automoc(${kdevgenericimporter_PART_SRCS})
kde4_add_plugin(kdevgenericimporter ${kdevgenericimporter_PART_SRCS})
target_link_libraries(kdevgenericimporter ${QT_AND_KDECORE_LIBS} kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevgenericimporter)
install_targets(${PLUGIN_INSTALL_DIR} kdevgenericimporter)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevgenericimporter.desktop)


kde4_footer()
