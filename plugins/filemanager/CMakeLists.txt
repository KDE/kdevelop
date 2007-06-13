include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
)


########### next target ###############

set(kdevfilemanager_PART_SRCS
    kdevfilemanagerpart.cpp
    filemanager.cpp
    drilldownview.cpp
    kdevdirmodel.cpp
    filesortfilterproxymodel.cpp
)

kde4_automoc(${kdevfilemanager_PART_SRCS})
kde4_add_plugin(kdevfilemanager ${kdevfilemanager_PART_SRCS})
target_link_libraries(kdevfilemanager ${KDE4_KDECORE_LIBS} kdevplatforminterfaces)

install(TARGETS kdevfilemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevfilemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevfilemanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevfilemanager )


