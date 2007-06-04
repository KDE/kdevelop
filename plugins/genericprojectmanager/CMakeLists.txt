
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/interfaces
    ${CMAKE_SOURCE_DIR}/src/project
    ${CMAKE_SOURCE_DIR}/src/project/interfaces
    )


########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kde4_automoc(${kdevgenericmanager_PART_SRCS})
kde4_add_plugin(kdevgenericmanager ${kdevgenericmanager_PART_SRCS})
target_link_libraries(kdevgenericmanager ${KDE4_KDECORE_LIBS} kdevplatforminterfaces kdevplatformproject )

install(TARGETS kdevgenericmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevgenericmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


