
kde4_add_executable(kdev_dbus_socket_transformer main.cpp)
target_link_libraries(kdev_dbus_socket_transformer ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
install(TARGETS kdev_dbus_socket_transformer ${INSTALL_TARGETS_DEFAULT_ARGS} )
