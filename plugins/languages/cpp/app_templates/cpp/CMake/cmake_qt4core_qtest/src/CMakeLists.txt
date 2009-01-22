include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(%{APPNAME}_SRCS %{APPNAME}.cpp main.cpp)
qt4_automoc(${%{APPNAME}_SRCS})
add_executable(%{APPNAME} ${%{APPNAME}_SRCS})
target_link_libraries(%{APPNAME} ${QT_QTCORE_LIBRARY})
