include_directories(
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake/parser
    ${KDEVPLATFORM_INCLUDE_DIR}
)

configure_file("paths.h.cmake" "cmake-test-paths.h" ESCAPE_QUOTES)

set(CMAKE_TEST_LIBRARIES 
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4cmakecommon
)

kde4_add_unit_test(cmake-parsertest cmakeparsertest.cpp)
target_link_libraries( cmake-parsertest ${CMAKE_TEST_LIBRARIES})

kde4_add_unit_test(cmakecompliance cmakecompliance.cpp)
target_link_libraries( cmakecompliance ${CMAKE_TEST_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES})

kde4_add_unit_test(cmake-astfactorytest astfactorytest.cpp)
target_link_libraries( cmake-astfactorytest ${CMAKE_TEST_LIBRARIES})

kde4_add_unit_test(cmakeast-test cmakeast_test.cpp)
target_link_libraries( cmakeast-test ${CMAKE_TEST_LIBRARIES})

kde4_add_unit_test(cmake-cmakecondition cmake_cmakecondition_test.cpp)
target_link_libraries( cmake-cmakecondition ${CMAKE_TEST_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS})
  
kde4_add_unit_test( cmakeduchaintest cmakeduchaintest.cpp)
target_link_libraries( cmakeduchaintest ${CMAKE_TEST_LIBRARIES}
${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS} ${KDEVPLATFORM_TESTS_LIBRARIES})

kde4_add_unit_test( cmakeprojectvisitortest cmake_cmakeprojectvisitor_test.cpp)
target_link_libraries( cmakeprojectvisitortest ${CMAKE_TEST_LIBRARIES}
${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS}  ${KDEVPLATFORM_TESTS_LIBRARIES})

kde4_add_unit_test( cmakeparserutilstest cmakeparserutilstest.cpp)
target_link_libraries( cmakeparserutilstest ${CMAKE_TEST_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS})

kde4_add_unit_test( cmakeloadprojecttest cmakeloadprojecttest.cpp )
target_link_libraries( cmakeloadprojecttest ${CMAKE_TEST_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

kde4_add_unit_test( cmakemanagertest cmakemanagertest.cpp )
target_link_libraries( cmakemanagertest ${CMAKE_TEST_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES})

kde4_add_unit_test( ctestfindsuitestest ctestfindsuitestest.cpp )
target_link_libraries( ctestfindsuitestest ${CMAKE_TEST_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

kde4_add_unit_test(generationexpressionsolvertest generationexpressionsolvertest.cpp)
target_link_libraries(generationexpressionsolvertest ${CMAKE_TEST_LIBRARIES})