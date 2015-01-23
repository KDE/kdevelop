include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(test_compilerprovider_SRCS
    test_compilerprovider.cpp
    ../../tests/projectsgenerator.cpp
)

ecm_add_test(${test_compilerprovider_SRCS}
    TEST_NAME test_compilerprovider
    LINK_LIBRARIES kdevcompilerprovider KDev::Tests Qt5::Test)
