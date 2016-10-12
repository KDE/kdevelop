ecm_add_test(test_cppcheckparser.cpp ../parser.cpp ../debug.cpp
    TEST_NAME test_cppcheckparser
    LINK_LIBRARIES Qt5::Test KDev::Tests)

ecm_add_test(test_cppcheckjob.cpp ../job.cpp ../parser.cpp ../debug.cpp
    TEST_NAME test_cppcheckjob
    LINK_LIBRARIES Qt5::Test KDev::Tests)
