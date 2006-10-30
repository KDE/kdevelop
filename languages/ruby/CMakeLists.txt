add_subdirectory(parser)

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/project )


########### next target ###############

set(kdevrubylanguagesupport_PART_SRCS
    rubylanguagesupport.cpp
    parsejob.cpp
)

kde4_automoc(${kdevrubylanguagesupport_PART_SRCS})
kde4_add_plugin(kdevrubylanguagesupport ${kdevrubylanguagesupport_PART_SRCS})
target_link_libraries(kdevrubylanguagesupport ${KDE4_KDECORE_LIBS} kdevplatform ${KDE4_THREADWEAVER_LIBRARIES} kdevrubyparser)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevrubylanguagesupport)
install(TARGETS kdevrubylanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevrubysupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
