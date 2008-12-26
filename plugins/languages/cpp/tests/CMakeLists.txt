set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_SOURCE_DIR}/plugins/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/plugins/languages/cpp
    ${CMAKE_BINARY_DIR}/plugins/languages/cpp
)

########### next target ###############

add_definitions(-DTEST_COMPLETION)

set(cppcodecompletiontest_SRCS test_cppcodecompletion.cpp ../codecompletioncontext.cpp ../stringhelpers.cpp ../completionitem.cpp ../completionhelpers.cpp ../implementationhelperitem.cpp ../cpplanguagesupport.cpp ../cpphighlighting.cpp ../cppcodecompletionmodel.cpp ../preprocessjob.cpp ../quickopen.cpp ../codegen/simplerefactoring.cpp ../cppparsejob.cpp ../includepathresolver.cpp ../setuphelpers.cpp ../codegen/cppnewclass.cpp ../codegen/progressdialogs.cpp ../cppcodecompletionworker.cpp ../includepathcomputer.cpp )


kde4_add_unit_test(cppcodecompletiontest ${cppcodecompletiontest_SRCS})
target_link_libraries(cppcodecompletiontest ${QT_QTTEST_LIBRARY}
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    veritascpp
    ${QT_QTDESIGNER_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDE4_KHTML_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)

