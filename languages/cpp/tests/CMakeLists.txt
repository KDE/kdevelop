set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/languages/cpp
    ${CMAKE_BINARY_DIR}/languages/cpp
)

########### next target ###############

add_definitions(-DTEST_COMPLETION)

set(cppcodecompletiontest_SRCS
  test_cppcodecompletion.cpp
  
  ../codecompletion/context.cpp
  ../stringhelpers.cpp
  ../codecompletion/item.cpp
  ../codecompletion/helpers.cpp
  ../codecompletion/implementationhelperitem.cpp
  ../cpplanguagesupport.cpp
  ../cpphighlighting.cpp
  ../codecompletion/model.cpp
  ../preprocessjob.cpp
  ../quickopen.cpp
  ../codegen/simplerefactoring.cpp
  ../cppparsejob.cpp
  ../includepathresolver.cpp
  ../setuphelpers.cpp
  ../codegen/cppnewclass.cpp
  ../codegen/progressdialogs.cpp
  ../codegen/unresolvedincludeassistant.cpp
  ../codegen/makeimplementationprivate.cpp
  ../codegen/cpputils.cpp
  ../codecompletion/worker.cpp
  ../includepathcomputer.cpp
  ../codecompletion/missingincludemodel.cpp
  ../codecompletion/missingincludeitem.cpp
)

set(cppcodegentest_SRCS
  test_cppcodegen.cpp
  
  ../cpphighlighting.cpp
  ../cpplanguagesupport.cpp
  ../cppparsejob.cpp
  ../includepathcomputer.cpp
  ../includepathresolver.cpp
  ../preprocessjob.cpp
  ../setuphelpers.cpp
  ../stringhelpers.cpp
  ../quickopen.cpp
  ../codecompletion/context.cpp
  ../codecompletion/helpers.cpp
  ../codecompletion/implementationhelperitem.cpp
  ../codecompletion/item.cpp
  ../codecompletion/missingincludeitem.cpp
  ../codecompletion/missingincludemodel.cpp
  ../codecompletion/model.cpp
  ../codecompletion/worker.cpp
  ../codegen/cppnewclass.cpp
  ../codegen/cpputils.cpp
  ../codegen/makeimplementationprivate.cpp
  ../codegen/progressdialogs.cpp
  ../codegen/simplerefactoring.cpp
  ../codegen/unresolvedincludeassistant.cpp
)


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
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)

kde4_add_unit_test(cppcodegentest ${cppcodegentest_SRCS})
target_link_libraries(cppcodegentest
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    veritascpp
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_SHELL_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARY}
)

