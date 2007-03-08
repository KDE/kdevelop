add_subdirectory(parser)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_BINARY_DIR}/parser
)


########### next target ###############

set(kdevrubylanguagesupport_PART_SRCS
    rubylanguagesupport.cpp
#    parsejob.cpp
)

kde4_automoc(${kdevrubylanguagesupport_PART_SRCS})
kde4_add_plugin(kdevrubylanguagesupport ${kdevrubylanguagesupport_PART_SRCS})
target_link_libraries(kdevrubylanguagesupport
    ${QT_QTDESIGNER_LIBRARY}
    kdevplatforminterfaces
    kdevrubyparser
)

install(TARGETS kdevrubylanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevrubysupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
