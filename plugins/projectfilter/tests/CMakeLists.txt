set(test_projectfilter_SRCS test_projectfilter.cpp
    ../projectfilter.cpp
    ../filter.cpp)

ecm_add_test(${test_projectfilter_SRCS}
    TEST_NAME test_projectfilter
    LINK_LIBRARIES ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${QT_QTTEST_LIBRARIES})
