
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9514 )


########### next target ###############

set(kdevduchainview_PART_SRCS
    duchainmodel.cpp
    duchainviewplugin.cpp
    duchaintree.cpp
)

kde4_add_plugin(kdevduchainview ${kdevduchainview_PART_SRCS})
target_link_libraries(kdevduchainview kdevplatformlanguage kdevplatforminterfaces ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS})

install(TARGETS kdevduchainview DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevduchainview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevduchainview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevduchainview)
