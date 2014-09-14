include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

add_executable(qml-parser
    qml-parser.cpp
)
target_link_libraries(qml-parser
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljsduchain
)

########### next target ###############

find_package(QJSON)
if(QJSON_FOUND AND KDEVPLATFORM_JSONTESTS_LIBRARIES)
  set(filestest_SRCS test_files.cpp)
  add_executable(filestest ${filestest_SRCS})
  add_test(filestest filestest)
  configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
  target_link_libraries(filestest
      ${QT_QTTEST_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
      ${KDEVPLATFORM_TESTS_LIBRARIES}
      ${KDEVPLATFORM_JSONTESTS_LIBRARIES}
      kdevqmljsduchain
  )
endif()
