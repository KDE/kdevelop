
include_directories(${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib
    )


########### next target ###############

set(kdevprojectmanager_PART_SRCS
    kdevprojectmanager_part.cpp
    kdevprojectmanager.cpp
    kdevprojectfilter.cpp
    kdevprojectmanagerdelegate.cpp
    importprojectjob.cpp
    importsourcefile_job.cpp
)

kde4_automoc(${kdevprojectmanager_PART_SRCS})
kde4_add_plugin(kdevprojectmanager ${kdevprojectmanager_PART_SRCS})
target_link_libraries(kdevprojectmanager ${KDE4_KDECORE_LIBS} kdevplatform ThreadWeaver kdevutil kdevwidgets)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevprojectmanager)
install(TARGETS kdevprojectmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevprojectmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevprojectmanager )


