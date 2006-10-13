include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets )


########### next target ###############

set(kdevfilemanager_PART_SRCS
    kdevfilemanagerpart.cpp
    filemanager.cpp
    drilldownview.cpp
    kdevdirmodel.cpp
)

kde4_automoc(kdevfilemanager ${kdevfilemanager_PART_SRCS})
kde4_add_plugin(kdevfilemanager ${kdevfilemanager_PART_SRCS})
target_link_libraries(kdevfilemanager ${KDE4_KDECORE_LIBS} kdevplatform kdevutil)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevfilemanager)
install(TARGETS kdevfilemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevfilemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevfilemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevfilemanager )


