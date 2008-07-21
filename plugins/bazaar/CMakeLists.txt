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

set(kdevbzr_PART_SRCS
    bzrplugin.cpp
    bzrexecutor.cpp
)

kde4_add_plugin(kdevbzr ${kdevbzr_PART_SRCS})


target_link_libraries(kdevbzr
    ${KDE4_KDEUI_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformproject
)

install(TARGETS kdevbzr DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevbzr.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevbzr.rc DESTINATION ${DATA_INSTALL_DIR}/kdevbzr )

