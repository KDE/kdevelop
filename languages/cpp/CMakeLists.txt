
add_subdirectory(parser)
add_subdirectory(app_templates)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/weaver ${CMAKE_SOURCE_DIR}/lib/editor
    )


########### next target ###############

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    parsejob.cpp
    codedelegate.cpp
    codeproxy.cpp
    cpphighlighting.cpp
)

kde4_automoc(${kdevcpplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport ${KDE4_KDECORE_LIBS} kdevplatform kdevcppparser ThreadWeaver kdevcpprpp)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcpplanguagesupport)
install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


