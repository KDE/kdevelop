project(unittests)
enable_testing()

add_executable(test_success success.cpp)
add_test(success test_success)

add_executable(test_fail fail.cpp)
set_target_properties(test_fail PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(fail test_fail)

add_executable(four_test math_test.cpp)
add_test(test_three four_test 3)
add_test(test_four four_test 4)
add_test(test_five four_test 5)