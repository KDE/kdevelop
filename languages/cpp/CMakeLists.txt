kde4_header()

add_subdirectory(parser)
add_subdirectory(app_templates) 

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/weaver
    )


########### next target ###############

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    cppsupportfactory.cpp
    backgroundparser.cpp
    parsejob.cpp
    codedelegate.cpp
    codeproxy.cpp
    cpphighlighting.cpp
)

kde4_automoc(${kdevcpplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport ${KDE4_KDECORE_LIBS} kdevinterfaces kdevcppparser ThreadWeaver kdevcpprpp)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcpplanguagesupport)
install_targets(${PLUGIN_INSTALL_DIR} kdevcpplanguagesupport)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevcppsupport.desktop)


kde4_footer()
