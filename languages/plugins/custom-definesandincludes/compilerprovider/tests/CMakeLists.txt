set(test_compilerprovider_SRCS
    test_compilerprovider.cpp
    ../icompiler.cpp
)
ecm_add_test(${test_compilerprovider_SRCS}
    TEST_NAME test_compilerprovider
    LINK_LIBRARIES KDev::Tests Qt5::Test)
