cmake_minimum_required(VERSION 3.10)

project(unittests)
enable_testing()

add_executable(test_success success.cpp)
add_test(success test_success)

add_executable(test_fail fail.cpp)
set_target_properties(test_fail PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(fail test_fail)
set_tests_properties(fail PROPERTIES
                     FOO "foo"
                     BAR TRUE
                     MULTILINE "this is 
a multi
line property"
                     QUOTES "\"\\\\\"\\\\\\"
                     WORKING_DIRECTORY "/bar/baz"
                     WILL_FAIL TRUE)

add_executable(four_test math_test.cpp)
add_test(NAME test_three COMMAND four_test 3 WORKING_DIRECTORY "/foo")
add_test(test_four four_test 4)

add_subdirectory(five)
