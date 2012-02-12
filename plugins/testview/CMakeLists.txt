
# add_definitions( -DKDE_DEFAULT_DEBUG_AREA=95xx )

########### next target ###############

set(kdevtestview_PLUGIN_SRCS
    testview.cpp
    testviewplugin.cpp
)

kde4_add_plugin(kdevtestview ${kdevtestview_PLUGIN_SRCS})
target_link_libraries(kdevtestview kdevplatforminterfaces kdevplatformproject kdevplatformutil)

install(TARGETS kdevtestview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevtestview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevtestview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevtestview )



