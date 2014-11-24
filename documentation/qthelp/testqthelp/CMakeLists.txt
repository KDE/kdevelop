project(testqthelp)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 4.5.0 COMPONENTS QtCore QtGui QtHelp QtWebKit REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
set(testqthelp_SRCS docwidget.cpp main.cpp)
ki18n_wrap_ui(testqthelp_SRCS docwidget.ui)
qt4_automoc(${testqthelp_SRCS})
add_executable(testqthelp ${testqthelp_SRCS})
target_link_libraries(testqthelp Qt5::Core Qt5::Gui Qt5::Help)
