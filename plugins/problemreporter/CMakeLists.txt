
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

set(kdevproblemreporter_PART_SRCS
    problemreporterplugin.cpp
    problemwidget.cpp
    problemmodel.cpp
#modeltest.cpp
)

kde4_add_plugin(kdevproblemreporter ${kdevproblemreporter_PART_SRCS})
target_link_libraries(kdevproblemreporter kdevplatformlanguage kdevplatforminterfaces kdevplatformeditor)

install(TARGETS kdevproblemreporter DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevproblemreporter.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevproblemreporter.rc DESTINATION ${DATA_INSTALL_DIR}/kdevproblemreporter)
