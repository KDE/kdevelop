remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(
    test_cppcheckparser.cpp

    TEST_NAME test_cppcheckparser
    LINK_LIBRARIES kdevcppcheck_core Qt::Test KDev::Tests
)

ecm_add_test(
    test_cppcheckjob.cpp

    TEST_NAME test_cppcheckjob
    LINK_LIBRARIES kdevcppcheck_core Qt::Test KDev::Tests
)
