include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp
    ${KDevelop_BINARY_DIR}/languages/cpp
)

set(setuphelpers_SRCS

  ../setuphelpers.cpp
)

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
  ../cpphighlighting.cpp
  ../cpputils.cpp
  ../includepathcomputer.cpp
  ../quickopen.cpp

  ${setuphelpers_SRCS}
)

ecm_add_test(test_buddies.cpp
LINK_LIBRARIES
    Qt5::Test
    Qt5::Widgets
    KDev::Language
    KDev::Project
    KDev::Tests
    KDev::Sublime
)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
ecm_add_test(test_cppfiles.cpp
LINK_LIBRARIES
    Qt5::Test
    Qt5::Core
    KDev::Language
    KDev::Tests
    KDev::JsonTests
    kdevcppduchain
    kdevcppparser
)

ecm_add_test(test_specialcompletion.cpp ${test_common_SRCS} TEST_NAME test_specialcompletion
LINK_LIBRARIES
    kdevcppduchain
    kdevcpprpp
    kdevcppparser
    Qt5::Test
    Qt5::Core
    KDev::Language
    KDev::Project
    KDev::Tests
)

########### next target ###############

set(test_cppassistants_SRCS
  ${test_common_SRCS}

  test_cppassistants.cpp
  ../codegen/adaptsignatureassistant.cpp
  ../codegen/adaptsignatureaction.cpp
)

ecm_add_test(${test_cppassistants_SRCS} TEST_NAME test_cppassistants
LINK_LIBRARIES
    Qt5::Test
    kdevcppduchain
    kdevcpprpp
    kdevcppparser
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Tests

    KF5::ThreadWeaver
    KF5::TextEditor
)

add_definitions(-DTEST_COMPLETION)

set(test_cppcodecompletion_SRCS
  test_cppcodecompletion.cpp ${test_common_SRCS}
)

set(test_cppcodegen_SRCS
  test_cppcodegen.cpp

  ../codegen/simplerefactoring.cpp
  ../codegen/unresolvedincludeassistant.cpp
  ../cpputils.cpp
  ../includepathcomputer.cpp
  ${setuphelpers_SRCS}
)

add_definitions(-DBUILD_TESTS)

ecm_add_test(${test_cppcodecompletion_SRCS} TEST_NAME test_cppcodecompletion
LINK_LIBRARIES
    Qt5::Test
    kdevcppduchain
    kdevcpprpp
    kdevcppparser
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Tests

    KF5::ThreadWeaver
    KF5::TextEditor
)

ecm_add_test(${test_cppcodegen_SRCS} TEST_NAME test_cppcodegen
LINK_LIBRARIES
    kdevcppduchain
    kdevcpprpp
    kdevcppparser
    Qt5::Test
    Qt5::Core
    KDev::Language
    KDev::Project
    KDev::Tests
)

add_executable( cpp-parser cpp-parser.cpp )
ecm_mark_as_test(cpp-parser)
target_link_libraries(cpp-parser
    Qt5::Core KDev::Tests KDev::Language
    kdevcppparser kdevcppduchain kdevcpprpp KF5::KDELibs4Support)
