
add_subdirectory(parser)
add_subdirectory(duchainbuilder)
add_subdirectory(app_templates)
add_subdirectory(expressionparser)
add_subdirectory(tests)

include_directories(${CMAKE_TOP_BUILD_DIR} ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/duchainbuilder
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/language
    ${KDEVPLATFORM_INCLUDE_DIR}/language/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/language/backgroundparser
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain/viewer
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

if(NOT WIN32)
set(kdevcpplanguagesupport_PART_SRCS
    ${kdevcpplanguagesupport_PART_SRCS}
    includepathresolver.cpp
    codecompletioncontext.cpp
    stringhelpers.cpp
    setuphelpers.cpp
)
endif(NOT WIN32)

kde4_automoc(${kdevcpplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport ${QT_QTDESIGNER_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY} kdevcppduchainbuilder kdevcppexpressionparser kdevcpprpp
    ${KDEVPLATFORM_PROJECT_LIBRARY} ${KDE4_THREADWEAVER_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARY}
)

install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
