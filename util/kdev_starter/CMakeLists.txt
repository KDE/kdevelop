
kde4_add_executable(kdev_starter main.cpp)
target_link_libraries(kdev_starter ${QT_QTCORE_LIBRARY} ${KDEWIN_LIBRARIES})
install(TARGETS kdev_starter ${INSTALL_TARGETS_DEFAULT_ARGS} )
