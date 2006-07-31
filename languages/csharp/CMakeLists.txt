
add_subdirectory(parser)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/weaver
    )


########### next target ###############

set(kdevcsharplanguagesupport_PART_SRCS
    csharplanguagesupport.cpp
    parsejob.cpp
#     codedelegate.cpp
#     codeproxy.cpp
)

kde4_automoc(${kdevcsharplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcsharplanguagesupport ${kdevcsharplanguagesupport_PART_SRCS})
target_link_libraries(kdevcsharplanguagesupport ${KDE4_KDECORE_LIBS} kdevplatform ThreadWeaver kdevcsharpparser )
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcsharplanguagesupport)
install(TARGETS kdevcsharplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcsharpsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


