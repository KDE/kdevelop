include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(test_highlighting_SRCS test_highlighting.cpp)

add_executable(test_highlighting ${test_highlighting_SRCS})
add_test(test_highlighting test_highlighting)
ecm_mark_as_test(test_highlighting)
target_link_libraries(test_highlighting KF5::TextEditor
    Qt5::Test 
    ${KDEVPLATFORM_TESTS_LIBRARIES} KDev::Language)
