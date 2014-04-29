include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(%{APPNAME}Test_SRCS %{APPNAME}Test.cpp)

qt4_automoc(${%{APPNAME}Test_SRCS})
add_executable(%{APPNAME}Test ${%{APPNAME}Test_SRCS})
add_test(%{APPNAME}Test %{APPNAME}Test)
target_link_libraries(%{APPNAME}Test ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
