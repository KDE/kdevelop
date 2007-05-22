
add_subdirectory(parser)
add_subdirectory(duchainbuilder)
add_subdirectory(app_templates)

include_directories(${CMAKE_TOP_BUILD_DIR} ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/editor
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/duchainbuilder
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/backgroundparser
    ${CMAKE_SOURCE_DIR}/lib/language/duchain
    ${CMAKE_SOURCE_DIR}/lib/language/duchain/viewer
)


########### next target ###############

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    cppparsejob.cpp
    preprocessjob.cpp
    cpphighlighting.cpp
    cppcodecompletion.cpp
    cppcodecompletionmodel.cpp
)

kde4_automoc(${kdevcpplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport ${QT_QTDESIGNER_LIBRARY}
    kdevplatforminterfaces kdevduchain kdevcppduchainbuilder kdevcpprpp kdevplatformproject kdevplatformlanguage)

install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
