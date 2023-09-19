remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_outputmodel.cpp LINK_LIBRARIES
    Qt::Test
    KDev::Tests
    KDev::OutputView
)

ecm_add_test(test_filteringstrategy.cpp LINK_LIBRARIES
    Qt::Test
    KDev::Tests
    KDev::OutputView
)

