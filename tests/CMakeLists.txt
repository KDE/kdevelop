include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

kde4_add_executable(clang-parser
    clang-parser.cpp
)
target_link_libraries(clang-parser
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevclangduchain
)

kde4_add_unit_test(test_buddies
    test_buddies.cpp
)
target_link_libraries(test_buddies
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
)

kde4_add_unit_test(test_codecompletion
    test_codecompletion.cpp
    $<TARGET_OBJECTS:kdevclangcodecompletion>
    $<TARGET_OBJECTS:kdevclangutil>
)
target_link_libraries(test_codecompletion
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    kdevclangduchain
)

kde4_add_unit_test(test_duchain
    test_duchain.cpp
)
target_link_libraries(test_duchain
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    kdevclangduchain
)

kde4_add_unit_test(test_problems
    test_problems.cpp
)
target_link_libraries(test_problems
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    kdevclangduchain
)

if(KDEVPLATFORM_JSONTESTS_LIBRARIES)
  set(filestest_SRCS test_files.cpp)
  kde4_add_unit_test(test_files ${filestest_SRCS})
  configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
  target_link_libraries(test_files
      ${QT_QTTEST_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
      ${KDEVPLATFORM_TESTS_LIBRARIES}
      ${KDEVPLATFORM_JSONTESTS_LIBRARIES}
      kdevclangduchain
  )
endif()
