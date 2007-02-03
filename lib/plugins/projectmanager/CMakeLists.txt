
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${THREADWEAVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    )


########### next target ###############

set(kdevprojectmanager_PART_SRCS
    projectmanager_part.cpp
    projectmanager.cpp
    projecttreeview.cpp
    projectmanagerdelegate.cpp
)

kde4_automoc(${kdevprojectmanager_PART_SRCS})
kde4_add_plugin(kdevprojectmanager ${kdevprojectmanager_PART_SRCS})
target_link_libraries(kdevprojectmanager ${KDE4_KDECORE_LIBS} koncreteinterfaces koncreteproject ${KDE4_THREADWEAVER_LIBRARIES} )

install(TARGETS kdevprojectmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevprojectmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanager.rc DESTINATION ${DATA_INSTALL_DIR}/kdevprojectmanager )


