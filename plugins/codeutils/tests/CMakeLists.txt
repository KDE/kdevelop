
configure_file(codeutils_tests_config.h.cmake codeutils_tests_config.h)

add_subdirectory(data/testgenerationtest/templates)

# add_executable(testgenerationtest testgenerationtest.cpp)
# ecm_mark_as_test(testgenerationtest)
# target_link_libraries(testgenerationtest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${QT_QTTEST_LIBRARIES})
