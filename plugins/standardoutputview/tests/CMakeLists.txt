set(test_standardOutputView_SRCS
    test_standardoutputview.cpp
    ../outputwidget.cpp
    ../toolviewdata.cpp
    ../standardoutputview.cpp
)

ecm_add_test(${test_standardOutputView_SRCS}
    TEST_NAME test_standardoutputview
    LINK_LIBRARIES Qt5::Test KDev::Tests)
