add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9532 )

set(kdevopenwith_PART_SRCS
    openwithplugin.cpp
)

kde4_add_plugin(kdevopenwith ${kdevopenwith_PART_SRCS})
target_link_libraries(kdevopenwith kdevplatforminterfaces kdevplatformproject ${KDE4_KDEUI_LIBS})

install(TARGETS kdevopenwith DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevopenwith.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES kdevopenwithui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevopenwith)
