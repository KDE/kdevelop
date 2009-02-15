project(QTestCmakeExample)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
enable_testing()

include_directories(${QT_INCLUDES} ${CMAKE_BINARY_DIR})
set(moneytest_SRCS moneytest.cpp)
add_test(moneytest moneytest)
add_executable(moneytest ${moneytest_SRCS})
qt4_automoc(${moneytest_SRCS})
target_link_libraries(moneytest ${QT_QTTEST_LIBRARY})
