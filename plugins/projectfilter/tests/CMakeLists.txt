set(test_projectfilter_SRCS test_projectfilter.cpp
    ../projectfilter.cpp
    ../filter.cpp)

ecm_add_test(${test_projectfilter_SRCS}
    TEST_NAME test_projectfilter
    LINK_LIBRARIES KDev::Project KDev::Tests ${QT_QTTEST_LIBRARIES})
