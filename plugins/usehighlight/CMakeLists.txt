
include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/language
    ${CMAKE_SOURCE_DIR}/editor
)


########### next target ###############

set(kdevusehighlight_PART_SRCS
    usehighlight.cpp
)

kde4_add_plugin(kdevusehighlight ${kdevusehighlight_PART_SRCS})
target_link_libraries(kdevusehighlight kdevplatforminterfaces kdevplatformutil kdevplatformlanguage)

install(TARGETS kdevusehighlight DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevusehighlight.desktop DESTINATION ${SERVICES_INSTALL_DIR})
