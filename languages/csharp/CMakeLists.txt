
add_subdirectory(parser)

include_directories( ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    # and one of the following two, depending on whether the parser files
    # are generated or the SVN versions are used:
    ${CMAKE_CURRENT_BINARY_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/generated
    )


########### next target ###############

set(kdevcsharplanguagesupport_PART_SRCS
    csharplanguagesupport.cpp
    parsejob.cpp
    codedelegate.cpp
    codeproxy.cpp
)

kde4_automoc(${kdevcsharplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcsharplanguagesupport ${kdevcsharplanguagesupport_PART_SRCS})
target_link_libraries(kdevcsharplanguagesupport ${KDE4_KDECORE_LIBS} kdevplatform ${KDE4_THREADWEAVER_LIBRARIES} kdevcsharpparser )

install(TARGETS kdevcsharplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcsharpsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


