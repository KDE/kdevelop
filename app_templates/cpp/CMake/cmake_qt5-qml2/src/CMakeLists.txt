qt_add_resources(%{APPNAMELC}_SRCS resources.qrc)
add_executable(%{APPNAMELC} main.cpp ${%{APPNAMELC}_SRCS})
target_link_libraries(%{APPNAMELC} Qt${QT_MAJOR_VERSION}::Qml Qt${QT_MAJOR_VERSION}::Gui)

