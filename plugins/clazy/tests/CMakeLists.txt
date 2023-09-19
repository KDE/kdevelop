remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(
    test_clazyjob.cpp

    TEST_NAME test_clazyjob
    LINK_LIBRARIES kdevclazy_core Qt::Test KDev::Tests
)
