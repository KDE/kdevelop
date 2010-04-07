project(%{APPNAMEID})
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(%{APPNAMEID}_SRCS %{APPNAMEID}.cpp main.cpp)
qt4_automoc(${%{APPNAMEID}_SRCS})
add_executable(%{APPNAMEID} ${%{APPNAMEID}_SRCS})
target_link_libraries(%{APPNAMEID} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
