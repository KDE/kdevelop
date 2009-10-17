PROJECT(git)

find_package(KDE4 REQUIRED)

find_package(KDevPlatform REQUIRED)
include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${KDE4_INCLUDE_DIR}/threadweaver
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/duchain
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_BINARY_DIR}/parser
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9525)

add_subdirectory(tests)

set(kdevgit_PART_SRCS
    gitplugin.cpp
)

kde4_add_plugin(kdevgit ${kdevgit_PART_SRCS})


target_link_libraries(kdevgit
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_VCS_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
)

install(TARGETS kdevgit DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevgit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevgit.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgit )

