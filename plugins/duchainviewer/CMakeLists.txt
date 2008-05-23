
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9514 )
include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/language
    ${CMAKE_SOURCE_DIR}/language/interfaces
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/shell
)


########### next target ###############

set(kdevduchainview_PART_SRCS
    duchainmodel.cpp
    duchainviewplugin.cpp
    duchaintree.cpp
)

kde4_add_plugin(kdevduchainview ${kdevduchainview_PART_SRCS})
target_link_libraries(kdevduchainview kdevplatformlanguage kdevplatforminterfaces kdevplatformeditor)

install(TARGETS kdevduchainview DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevduchainview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevduchainview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevduchainview)
