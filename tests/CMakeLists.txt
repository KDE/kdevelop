ecm_add_test(test_plugin.cpp ../plugin.cpp ../job.cpp ../debug.cpp
    TEST_NAME plugin_tests
    LINK_LIBRARIES Qt5::Test KDev::Tests)

# ecm_add_test(test_cppcheckparser.cpp ../parsers/cppcheckparser.cpp ../debug.cpp
#     TEST_NAME test_cppcheckparser
#     LINK_LIBRARIES Qt5::Test KDev::Tests)
# 
# ecm_add_test(test_cppcheckjob.cpp ../job.cpp ../parsers/cppcheckparser.cpp ../debug.cpp
#     TEST_NAME test_cppcheckjob
#     LINK_LIBRARIES Qt5::Test KDev::Tests)
