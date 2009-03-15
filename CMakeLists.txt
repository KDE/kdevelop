add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9525)
add_subdirectory(tests)

########### next target ###############

set(kdevgit_PART_SRCS
    gitplugin.cpp
)

kde4_add_plugin(kdevgit ${kdevgit_PART_SRCS})


target_link_libraries(kdevgit
    ${KDE4_KDEUI_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformshell
    kdevplatformproject
)

install(TARGETS kdevgit DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevgit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevgit.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgit )

