project(qt4app)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(qt4app_SRCS qt4app.cpp main.cpp)
qt4_automoc(${qt4app_SRCS})
add_executable(qt4app ${qt4app_SRCS})
target_link_libraries(qt4app ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
