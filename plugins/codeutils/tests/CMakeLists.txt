
configure_file(codeutils_tests_config.h.cmake codeutils_tests_config.h)

add_subdirectory(data/testgenerationtest/templates)

# add_executable(testgenerationtest testgenerationtest.cpp)
# add_test(testgenerationtest testgenerationtest)
# ecm_mark_as_test(testgenerationtest)
# target_link_libraries(testgenerationtest KDev::Language KDev::Tests ${QT_QTTEST_LIBRARIES})
