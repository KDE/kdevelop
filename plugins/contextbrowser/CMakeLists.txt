
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9524 )
include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/language/editor
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/language
)


########### next target ###############

set(kdevcontextbrowser_PART_SRCS
    contextbrowser.cpp
    contextbrowserview.cpp
)

kde4_add_plugin(kdevcontextbrowser ${kdevcontextbrowser_PART_SRCS})
target_link_libraries(kdevcontextbrowser kdevplatforminterfaces kdevplatformutil kdevplatformlanguage)

install(TARGETS kdevcontextbrowser DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevcontextbrowser.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcontextbrowser)

########### install files ###############

install(FILES kdevcontextbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
