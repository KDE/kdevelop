include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

kde4_add_executable(clang-parser
    clang-parser.cpp
)
target_link_libraries(clang-parser
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevclangduchain
)

kde4_add_executable(clang-standalone-parser
    clang-standalone-parser.c
)
target_link_libraries(clang-standalone-parser
    ${CLANG_CLANG_LIB}
)

########### next target ###############

kde4_add_unit_test(codecompletiontest
    codecompletiontest.cpp
    $<TARGET_OBJECTS:kdevclangcodecompletion>
)

target_link_libraries(codecompletiontest
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    kdevclangduchain
)

########### next target ###############

kde4_add_unit_test(duchaintest
    duchaintest.cpp
)

target_link_libraries(duchaintest
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    kdevclangduchain
)

########### next target ###############
kde4_add_unit_test(problemstest
    test_problems.cpp
)

target_link_libraries(problemstest
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    kdevclangduchain
)

########### next target ###############

if(KDEVPLATFORM_JSONTESTS_LIBRARIES)
  set(filestest_SRCS test_files.cpp)
  kde4_add_unit_test(filestest ${filestest_SRCS})
  configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
  target_link_libraries(filestest
      ${QT_QTTEST_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
      ${KDEVPLATFORM_TESTS_LIBRARIES}
      ${KDEVPLATFORM_JSONTESTS_LIBRARIES}
      kdevclangduchain
  )
endif()
