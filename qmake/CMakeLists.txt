add_subdirectory(parser)
add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    )


########### next target ###############

set(kdevqmakeimporter_PART_SRCS
    qmakeimporter.cpp
    qmakemodelitems.cpp
    qmakeprojectscope.cpp
)

kde4_automoc(${kdevqmakeimporter_PART_SRCS})
kde4_add_plugin(kdevqmakeimporter ${kdevqmakeimporter_PART_SRCS})
target_link_libraries( kdevqmakeimporter ${KDE4_KDECORE_LIBS} kdevplatform kdevqmakeparser )

install(TARGETS kdevqmakeimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevqmakeimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


