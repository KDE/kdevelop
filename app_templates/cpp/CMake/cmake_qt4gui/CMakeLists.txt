cmake_minimum_required(VERSION 2.6)
project(%{APPNAMELC})
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(%{APPNAME}_SRCS %{APPNAME}.cpp main.cpp)

qt4_automoc(${%{APPNAME}_SRCS})
add_executable(%{APPNAMELC} ${%{APPNAME}_SRCS})
target_link_libraries(%{APPNAMELC} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS %{APPNAMELC} RUNTIME DESTINATION bin)
