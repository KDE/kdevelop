
# add_definitions( -DKDE_DEFAULT_DEBUG_AREA=95xx )

########### next target ###############

set(kdevtestcontroller_PLUGIN_SRCS
    testcontroller.cpp
)

kde4_add_plugin(kdevtestcontroller ${kdevtestcontroller_PLUGIN_SRCS})
target_link_libraries(kdevtestcontroller kdevplatforminterfaces kdevplatformproject  )

install(TARGETS kdevtestcontroller DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevtestcontroller.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


