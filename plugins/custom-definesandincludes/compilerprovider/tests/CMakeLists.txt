remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(test_compilerprovider_SRCS
    test_compilerprovider.cpp
)

ecm_add_test(${test_compilerprovider_SRCS}
    TEST_NAME test_compilerprovider
    LINK_LIBRARIES kdevcompilerprovider KDev::Tests Qt::Test)
