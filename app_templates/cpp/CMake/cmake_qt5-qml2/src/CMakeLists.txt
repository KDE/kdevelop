qt5_add_resources(%{APPNAMELC}_SRCS resources.qrc)
add_executable(%{APPNAMELC} main.cpp ${%{APPNAMELC}_SRCS})
target_link_libraries(%{APPNAMELC} Qt5::Qml Qt5::Gui)

