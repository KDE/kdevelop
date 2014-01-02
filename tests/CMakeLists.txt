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

find_package(QJSON)
if(QJSON_FOUND AND KDEVPLATFORM_JSONTESTS_LIBRARIES)
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
