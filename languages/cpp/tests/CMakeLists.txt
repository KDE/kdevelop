include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp
    ${KDevelop_BINARY_DIR}/languages/cpp
)

set(setuphelpers_SRCS

  ../setuphelpers.cpp
  ../setuphelpers_gcc.cpp
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
  ../codecompletion/missingincludemodel.cpp
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

ecm_add_test(test_buddies.cpp TEST_NAME buddiestest
LINK_LIBRARIES
    Qt5::Test
    Qt5::Widgets
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_SUBLIME_LIBRARIES}
)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
ecm_add_test(test_cppfiles.cpp
TEST_NAME cppfilestest
LINK_LIBRARIES
    Qt5::Test
    Qt5::Core
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_JSONTESTS_LIBRARIES}
    kdev4cppduchain
)

ecm_add_test(test_specialcompletion.cpp ${test_common_SRCS} TEST_NAME cppspecialcompletion
LINK_LIBRARIES
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    Qt5::Test
    Qt5::Core
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

ecm_add_test(${cppassistantstest_SRCS} TEST_NAME cppassistantstest
LINK_LIBRARIES
    Qt5::Test
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}

    KF5::ThreadWeaver
    KF5::TextEditor
)

add_definitions(-DTEST_COMPLETION)

set(cppcodecompletiontest_SRCS
  test_cppcodecompletion.cpp ${test_common_SRCS}
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

ecm_add_test(${cppcodecompletiontest_SRCS} TEST_NAME cppcodecompletiontest
TEST_NAME cppcodecompletiontest
LINK_LIBRARIES
    Qt5::Test
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}

    KF5::ThreadWeaver
    KF5::TextEditor
)

ecm_add_test(${cppcodegentest_SRCS} TEST_NAME cppcodegentest
LINK_LIBRARIES
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    Qt5::Test
    Qt5::Core
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)

add_executable( cpp-parser cpp-parser.cpp )
ecm_mark_as_test(cpp-parser)
target_link_libraries(cpp-parser
    Qt5::Core ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    kdev4cppparser kdev4cppduchain kdev4cpprpp KF5::KDELibs4Support)
