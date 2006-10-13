
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/lib/editor ${CMAKE_SOURCE_DIR}/languages/cpp/parser ${CMAKE_SOURCE_DIR}/languages/cpp/duchain
    )


########### next target ###############

set(kdevduchainview_PART_SRCS
    duchainmodel.cpp
    duchainview_part.cpp
)

kde4_automoc(kdevduchainview ${kdevduchainview_PART_SRCS})
kde4_add_plugin(kdevduchainview ${kdevduchainview_PART_SRCS})
target_link_libraries(kdevduchainview kdevcpplanguage )
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevduchainview)
install(TARGETS kdevduchainview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevduchainview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevduchainview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevduchainview )


