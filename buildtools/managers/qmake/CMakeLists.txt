add_subdirectory(parser)

add_subdirectory(tests)

include_directories(${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/buildtools/builders/qmakebuilder
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    )


########### next target ###############

set(kdevqmakemanager_PART_SRCS
    qmakemanager.cpp
    qmakemodelitems.cpp
    qmakeprojectscope.cpp
)

kde4_automoc(${kdevqmakemanager_PART_SRCS})
kde4_add_plugin(kdevqmakemanager ${kdevqmakemanager_PART_SRCS})
target_link_libraries( kdevqmakemanager ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} kdevqmakeparser )

install(TARGETS kdevqmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevqmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


