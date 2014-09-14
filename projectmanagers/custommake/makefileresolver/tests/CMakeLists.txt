set(test_custommake_SRCS
    test_custommake.cpp
    ../makefileresolver.cpp
)

ecm_add_test(${test_custommake_SRCS}
    TEST_NAME test_custommake
    LINK_LIBRARIES
        Qt5::Test
        KDev::Tests
        KDev::Project
)
