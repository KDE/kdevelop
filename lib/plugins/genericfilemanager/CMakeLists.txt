
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    )


########### next target ###############

set(kdevgenericimporter_PART_SRCS
    genericimporter.cpp
)

kde4_automoc(${kdevgenericimporter_PART_SRCS})
kde4_add_plugin(kdevgenericimporter ${kdevgenericimporter_PART_SRCS})
target_link_libraries(kdevgenericimporter ${KDE4_KDECORE_LIBS} kdevplatforminterfaces kdevplatformproject )

install(TARGETS kdevgenericimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevgenericimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


