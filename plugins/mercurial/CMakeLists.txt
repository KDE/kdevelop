add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9525)
add_subdirectory(tests)
########### next target ###############

set(kdevmercurial_PART_SRCS
    mercurialplugin.cpp
)

kde4_add_plugin(kdevmercurial ${kdevmercurial_PART_SRCS})

target_link_libraries(kdevmercurial
    ${KDE4_KDEUI_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformshell
    kdevplatformproject
)

install(TARGETS kdevmercurial DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevmercurial.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevmercurial.rc DESTINATION ${DATA_INSTALL_DIR}/kdevmercurial )

