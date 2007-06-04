
include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/interfaces
    ${CMAKE_SOURCE_DIR}/src/editor
    ${CMAKE_SOURCE_DIR}/src/language
    ${CMAKE_SOURCE_DIR}/src/language/interfaces
    ${CMAKE_SOURCE_DIR}/src/language/duchain
    ${CMAKE_SOURCE_DIR}/src/shell
)


########### next target ###############

set(kdevduchainview_PART_SRCS
    duchainmodel.cpp
    duchainview_part.cpp
)

kde4_automoc(${kdevduchainview_PART_SRCS})
kde4_add_plugin(kdevduchainview ${kdevduchainview_PART_SRCS})
target_link_libraries(kdevduchainview kdevplatformlanguage kdevplatforminterfaces kdevplatformeditor)

install(TARGETS kdevduchainview DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevduchainview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevduchainview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevduchainview)
