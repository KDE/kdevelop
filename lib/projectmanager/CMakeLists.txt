
include_directories(
    ${CMAKE_SOURCE_DIR}/lib/util
    ${CMAKE_SOURCE_DIR}/lib 
    ${THREADWEAVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/project
    )


########### next target ###############

set(kdevprojectmanager_PART_SRCS
    kdevprojectmanager_part.cpp
    kdevprojectmanager.cpp
    kdevprojectfilter.cpp
    kdevprojectmanagerdelegate.cpp
)

kde4_automoc(${kdevprojectmanager_PART_SRCS})
kde4_add_plugin(kdevprojectmanager ${kdevprojectmanager_PART_SRCS})
target_link_libraries(kdevprojectmanager ${KDE4_KDECORE_LIBS} kdevplatform ${KDE4_THREADWEAVER_LIBRARIES} kdevutil kdevwidgets)

install(TARGETS kdevprojectmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevprojectmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevprojectmanager )


