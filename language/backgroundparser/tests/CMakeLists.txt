include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(test_backgroundparser_SRCS test_backgroundparser.cpp testlanguagesupport.cpp testparsejob.cpp)

add_executable(test_backgroundparser ${test_backgroundparser_SRCS})
add_test(test_backgroundparser test_backgroundparser)
ecm_mark_as_test(test_backgroundparser)
target_link_libraries(test_backgroundparser KF5::TextEditor
    Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KF5::ThreadWeaver KDev::Language)
