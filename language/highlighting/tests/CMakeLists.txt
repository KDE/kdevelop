include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(highlightingtest_SRCS test_highlighting.cpp)

add_executable(highlightingtest ${highlightingtest_SRCS})
add_test(highlightingtest highlightingtest)
ecm_mark_as_test(highlightingtest)
target_link_libraries(highlightingtest KF5::TextEditor
    Qt5::Test 
    ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)
