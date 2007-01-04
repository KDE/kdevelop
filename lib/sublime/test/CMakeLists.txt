if (KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/lib)

set( areaoperationtest_SRCS areaoperationtest.cpp areaprinter.cpp )
kde4_automoc(${areaoperationtest_SRCS})
kde4_add_executable(sublime-areaoperationtest NOGUI RUN_UNINSTALLED ${areaoperationtest_SRCS})
target_link_libraries(sublime-areaoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )
add_test(sublime-areaoperationtest ${EXECUTABLE_OUTPUT_PATH}/sublime-areaoperationtest)

endif(KDE4_BUILD_TESTS)
