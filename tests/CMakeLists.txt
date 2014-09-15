include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(clang-parser
    clang-parser.cpp
)
target_link_libraries(clang-parser
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevclangduchain
)

ecm_add_test(test_buddies.cpp
    TEST_NAME test_buddies
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
)

ecm_add_test(test_codecompletion.cpp
    TEST_NAME test_codecompletion
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
        kdevclangcodecompletion
        kdevclangduchain
)

ecm_add_test(test_assistants.cpp
    TEST_NAME test_assistants
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
        kdevclangduchain
        kdevclangutil
)

ecm_add_test(test_clangutils.cpp
    TEST_NAME test_clangutils
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
        ${CLANG_CLANG_LIB}
        kdevclangduchain
)

ecm_add_test(test_duchain.cpp
    TEST_NAME test_duchain
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
        kdevclangduchain
)

ecm_add_test(test_duchainutils.cpp
    TEST_NAME test_duchainutils
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
        kdevclangcodecompletion
        kdevclangduchain
)

ecm_add_test(test_problems.cpp
    TEST_NAME test_problems
    LINK_LIBRARIES
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${QT_QTTEST_LIBRARY}
        kdevclangduchain
)

if(KDEVPLATFORM_JSONTESTS_LIBRARIES)
  configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
  ecm_add_test(test_files.cpp
    TEST_NAME test_files
    LINK_LIBRARIES
      ${QT_QTTEST_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
      ${KDEVPLATFORM_TESTS_LIBRARIES}
      ${KDEVPLATFORM_JSONTESTS_LIBRARIES}
      kdevclangduchain
  )
endif()
