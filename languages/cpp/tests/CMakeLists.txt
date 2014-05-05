set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp
    ${KDevelop_BINARY_DIR}/languages/cpp
)

set(setuphelpers_SRCS

  ../setuphelpers.cpp
  ../setuphelpers_gcc_like.cpp
)
if(MSVC)
    list(APPEND setuphelpers_SRCS ../setuphelpers_msvc.cpp)
endif(MSVC)

set(test_common_SRCS

  ../codecompletion/context.cpp
  ../codecompletion/helpers.cpp
  ../codecompletion/implementationhelperitem.cpp
  ../codecompletion/item.cpp
  ../codecompletion/missingincludeitem.cpp
  ../codecompletion/model.cpp
  ../codecompletion/worker.cpp
  ../codegen/simplerefactoring.cpp
  ../codegen/unresolvedincludeassistant.cpp
  ../codegen/customincludepaths.cpp
  ../cpphighlighting.cpp
  ../cpputils.cpp
  ../includepathcomputer.cpp
  ../includepathresolver.cpp
  ../quickopen.cpp

  ${setuphelpers_SRCS}
)

########### next target ###############

set(buddiestest_SRCS
  test_buddies.cpp
)

kde4_add_unit_test(buddiestest ${buddiestest_SRCS})
target_link_libraries(buddiestest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_SUBLIME_LIBRARIES}
)

########### next target ###############

# Also check that kdevplatform is built with JSON support
# see: https://bugs.kde.org/show_bug.cgi?id=327095
if(QJSON_FOUND AND KDEVPLATFORM_JSONTESTS_LIBRARIES)
  set(cppfilestest_SRCS test_cppfiles.cpp)
  kde4_add_unit_test(cppfilestest ${cppfilestest_SRCS})
  configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
  target_link_libraries(cppfilestest
      ${QT_QTTEST_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
      ${KDEVPLATFORM_TESTS_LIBRARIES}
      ${KDEVPLATFORM_JSONTESTS_LIBRARIES}
      kdev4cppduchain
  )
endif()

########### next target ###############

set(specialcompletiontest_SRCS
  test_specialcompletion.cpp

  ${test_common_SRCS}
)

kde4_add_unit_test(cppspecialcompletion ${specialcompletiontest_SRCS})
target_link_libraries(cppspecialcompletion
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)

########### next target ###############

set(cppassistantstest_SRCS
  ${test_common_SRCS}

  test_cppassistants.cpp
  ../codegen/renameassistant.cpp
  ../codegen/renameaction.cpp
  ../codegen/renamefileaction.cpp
  ../codegen/codeassistant.cpp
  ../codegen/signatureassistant.cpp
  ../codegen/adaptsignatureaction.cpp
)

kde4_add_unit_test(cppassistantstest ${cppassistantstest_SRCS})
target_link_libraries(cppassistantstest ${QT_QTTEST_LIBRARY}
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)

add_definitions(-DTEST_COMPLETION)

set(cppcodecompletiontest_SRCS
  test_cppcodecompletion.cpp

  ${test_common_SRCS}
)

set(cppcodegentest_SRCS
  test_cppcodegen.cpp

  ../codegen/simplerefactoring.cpp
  ../codegen/unresolvedincludeassistant.cpp
  ../codegen/customincludepaths.cpp
  ../cpputils.cpp
  ../includepathcomputer.cpp
  ../includepathresolver.cpp
  ${setuphelpers_SRCS}
)

add_definitions(-DBUILD_TESTS)

kde4_add_unit_test(cppcodecompletiontest ${cppcodecompletiontest_SRCS})
target_link_libraries(cppcodecompletiontest ${QT_QTTEST_LIBRARY}
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)

kde4_add_unit_test(cppcodegentest ${cppcodegentest_SRCS})
target_link_libraries(cppcodegentest
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)

kde4_add_executable( cpp-parser cpp-parser.cpp )
target_link_libraries(cpp-parser
${QT_QTCORE_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
kdev4cppparser kdev4cppduchain kdev4cpprpp)
