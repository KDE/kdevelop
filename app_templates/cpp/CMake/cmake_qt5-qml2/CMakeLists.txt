cmake_minimum_required(VERSION 3.0)

find_package(Qt5 REQUIRED COMPONENTS Gui Qml)

qt5_add_resources(%{APPNAMELC}_SRCS resources.qrc)
add_executable(%{APPNAMELC} src/main.cpp ${%{APPNAMELC}_SRCS})
target_link_libraries(%{APPNAMELC} Qt5::Qml Qt5::Gui)
