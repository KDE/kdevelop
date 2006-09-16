
add_subdirectory(parser)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${THREADWEAVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/project
    )


########### next target ###############

set(kdevjavalanguagesupport_PART_SRCS
    javalanguagesupport.cpp
    parsejob.cpp
#     codedelegate.cpp
#     codeproxy.cpp
)

kde4_automoc(${kdevjavalanguagesupport_PART_SRCS})
kde4_add_plugin(kdevjavalanguagesupport ${kdevjavalanguagesupport_PART_SRCS})
target_link_libraries(kdevjavalanguagesupport ${KDE4_KDECORE_LIBS} kdevplatform ThreadWeaver kdevjavaparser )
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevjavalanguagesupport)
install(TARGETS kdevjavalanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevjavasupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


