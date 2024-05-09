remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(test_definesandincludes_SRCS
    test_definesandincludes.cpp
)

ecm_add_test(${test_definesandincludes_SRCS}
    TEST_NAME test_definesandincludes
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Project)
