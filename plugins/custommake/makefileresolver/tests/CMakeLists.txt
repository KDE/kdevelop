remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

set(test_custommake_SRCS
    test_custommake.cpp
)
ecm_add_test(${test_custommake_SRCS}
    TEST_NAME test_custommake
    LINK_LIBRARIES
        Qt::Test
        KDev::Tests
        KDev::Project
        kdevmakefileresolver
)
