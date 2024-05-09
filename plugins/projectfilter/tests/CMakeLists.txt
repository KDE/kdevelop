remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

set(test_projectfilter_SRCS test_projectfilter.cpp
    ../projectfilter.cpp
    ../filter.cpp)

ecm_add_test(${test_projectfilter_SRCS}
    TEST_NAME test_projectfilter
    LINK_LIBRARIES KDev::Project KDev::Tests Qt::Test)
