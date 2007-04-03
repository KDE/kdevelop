
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/plugins/vcs/interfaces
    ${THREADWEAVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    )


########### next target ###############

set(kdevprojectmanagerview_PART_SRCS
    projectmanagerview_part.cpp
    projectmanagerview.cpp
    projecttreeview.cpp
    projectmanagerdelegate.cpp
)

kde4_automoc(${kdevprojectmanagerview_PART_SRCS})
kde4_add_plugin(kdevprojectmanagerview ${kdevprojectmanagerview_PART_SRCS})
target_link_libraries(kdevprojectmanagerview ${KDE4_KDECORE_LIBS} kdevplatforminterfaces kdevplatformproject ${KDE4_THREADWEAVER_LIBRARIES} )

install(TARGETS kdevprojectmanagerview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevprojectmanagerview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevprojectmanagerview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevprojectmanagerview )


