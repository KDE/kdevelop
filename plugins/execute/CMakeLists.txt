
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

########### next target ###############

set(kdevexecute_PART_SRCS
    executeplugin.cpp
)

kde4_add_plugin(kdevexecute ${kdevexecute_PART_SRCS})
target_link_libraries(kdevexecute ${KDE4_KDEUI_LIBS} kdevplatforminterfaces kdevplatformutil)

install(TARGETS kdevexecute DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevexecute.desktop DESTINATION ${SERVICES_INSTALL_DIR})
