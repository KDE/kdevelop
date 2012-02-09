project(ctestfinder)
#TODO: add_definitions( -DKDE_DEFAULT_DEBUG_AREA=90xx )

########### next target ###############

set(ctestfinder_LIB_SRCS
    ctestfinder.cpp
    ctestlauncher.cpp
    ctestsuite.cpp
)

kde4_add_plugin(kdevctestfinder ${ctestfinder_LIB_SRCS})
target_link_libraries(
    kdevctestfinder
        ${KDE4_KIO_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

install(TARGETS kdevctestfinder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kdevctestfinder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


