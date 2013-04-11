
configure_file(codeutils_tests_config.h.cmake codeutils_tests_config.h)

add_subdirectory(data/testgenerationtest/templates)

kde4_add_unit_test(testgenerationtest testgenerationtest.cpp)
target_link_libraries(testgenerationtest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${QT_QTTEST_LIBRARIES})
