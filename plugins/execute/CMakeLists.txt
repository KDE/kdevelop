
include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/util
)


########### next target ###############

set(kdevexecute_PART_SRCS
    executeplugin.cpp
)

kde4_add_plugin(kdevexecute ${kdevexecute_PART_SRCS})
target_link_libraries(kdevexecute kdevplatforminterfaces kdevplatformutil)

install(TARGETS kdevexecute DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevexecute.desktop DESTINATION ${SERVICES_INSTALL_DIR})
