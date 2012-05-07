project(%{APPNAME})
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(%{APPNAME}_SRCS %{APPNAME}.cpp main.cpp)
qt4_automoc(${%{APPNAME}_SRCS})
add_executable(%{APPNAME} ${%{APPNAME}_SRCS})
target_link_libraries(%{APPNAME} ${QT_QTCORE_LIBRARY})

install(TARGETS %{APPNAME} RUNTIME DESTINATION bin)
