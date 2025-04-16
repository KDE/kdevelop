cmake_minimum_required(VERSION 3.10)
project(unittestskde)

set(CMAKE_CXX_STANDARD 17)
# For some reason, disabling CXX_EXTENSIONS prevents an assertion failure in
# ItemRepository::index() in a ClangParseJob::run() thread during
# TestCTestFindSuites::cleanup() or TestCTestFindSuites::cleanupTestCase():
# "found item in unexpected bucket, ensure your ItemRequest::equals method is correct. "
# "Note: For custom AbstractType's e.g. ensure you have a proper equals() override"
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Test)

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMAddTests)

enable_testing()
set(CMAKE_AUTOMOC ON)

ecm_add_test(test.cpp TEST_NAME cmake-test-unittestskde)
ecm_add_test(test_main.cpp TEST_NAME cmake-test-unittestskde-withmain)
target_link_libraries(cmake-test-unittestskde Qt::Test)
target_link_libraries(cmake-test-unittestskde-withmain Qt::Test)
