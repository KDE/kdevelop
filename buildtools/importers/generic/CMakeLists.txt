
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    )


########### next target ###############

set(kdevgenericimporter_PART_SRCS
    kdevgenericimporter.cpp
)

kde4_automoc(${kdevgenericimporter_PART_SRCS})
kde4_add_plugin(kdevgenericimporter ${kdevgenericimporter_PART_SRCS})
target_link_libraries(kdevgenericimporter ${KDE4_KDECORE_LIBS} kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevgenericimporter)
install(TARGETS kdevgenericimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevgenericimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


