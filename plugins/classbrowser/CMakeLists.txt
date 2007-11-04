
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

set(kdevclassbrowser_PART_SRCS
    classbrowserpart.cpp
    classmodel.cpp
    classtree.cpp
#modeltest.cpp
)

kde4_add_plugin(kdevclassbrowser ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser kdevplatformlanguage kdevplatforminterfaces kdevplatformeditor)

install(TARGETS kdevclassbrowser DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevclassbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevclassbrowser.rc DESTINATION ${DATA_INSTALL_DIR}/kdevclassbrowser)
