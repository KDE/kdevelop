
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
    )


########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kde4_add_plugin(kdevgenericmanager ${kdevgenericmanager_PART_SRCS})
target_link_libraries(kdevgenericmanager ${KDE4_KDECORE_LIBS} kdevplatforminterfaces kdevplatformproject )

install(TARGETS kdevgenericmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevgenericmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


