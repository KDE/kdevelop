
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(%{APPNAMEID}Test_SRCS %{APPNAMEID}Test.cpp)
qt4_automoc(${%{APPNAMEID}Test_SRCS})
add_executable(%{APPNAMEID}Test ${%{APPNAMEID}Test_SRCS})
add_test(%{APPNAMEID}Test %{APPNAMEID}Test)
target_link_libraries(%{APPNAMEID}Test ${QT_QTTEST_LIBRARY})
