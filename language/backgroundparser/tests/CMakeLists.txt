include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(backgroundparsertest_SRCS test_backgroundparser.cpp testlanguagesupport.cpp testparsejob.cpp)

add_executable(backgroundparsertest ${backgroundparsertest_SRCS})
ecm_mark_as_test(backgroundparsertest)
target_link_libraries(backgroundparsertest KF5::TextEditor
    Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KF5::ThreadWeaver kdevplatformlanguage)
