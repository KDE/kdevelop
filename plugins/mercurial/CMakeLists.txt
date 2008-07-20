add_subdirectory(tests)

include_directories( ${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/vcs
    ${CMAKE_SOURCE_DIR}/vcs/interfaces
    ${CMAKE_SOURCE_DIR}/vcs/models
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/
)


########### next target ###############

set(kdevhg_PART_SRCS
    hgplugin.cpp
    hgexecutor.cpp
)

kde4_add_plugin(kdevhg ${kdevhg_PART_SRCS})

target_link_libraries(kdevhg
    ${KDE4_KDECORE_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformproject
)

install(TARGETS kdevhg DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevhg.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevhg.rc DESTINATION ${DATA_INSTALL_DIR}/kdevhg )

