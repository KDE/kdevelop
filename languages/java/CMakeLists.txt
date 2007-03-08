
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

set(kdevjavalanguagesupport_PART_SRCS
    javalanguagesupport.cpp
#    parsejob.cpp
#    codedelegate.cpp
#    codeproxy.cpp
)

kde4_automoc(${kdevjavalanguagesupport_PART_SRCS})
kde4_add_plugin(kdevjavalanguagesupport ${kdevjavalanguagesupport_PART_SRCS})
target_link_libraries(kdevjavalanguagesupport
    ${QT_QTDESIGNER_LIBRARY}
    kdevplatforminterfaces
    kdevjavaparser
)

install(TARGETS kdevjavalanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevjavasupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})


