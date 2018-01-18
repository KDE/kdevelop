cmake_minimum_required(VERSION 3.0)
project(unittestskde)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Test)

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMAddTests)

enable_testing()
set(CMAKE_AUTOMOC ON)

ecm_add_test(test.cpp TEST_NAME cmake-test-unittestskde)
target_link_libraries(cmake-test-unittestskde Qt5::Test)
