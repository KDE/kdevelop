remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(test_backgroundparser_SRCS test_backgroundparser.cpp testlanguagesupport.cpp testparsejob.cpp)
ecm_add_test(${test_backgroundparser_SRCS}
    TEST_NAME test_backgroundparser
    LINK_LIBRARIES KF5::TextEditor Qt5::Test KDev::Tests KF5::ThreadWeaver KDev::Language)
