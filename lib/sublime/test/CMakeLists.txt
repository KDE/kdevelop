if (KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/lib)

set( areaoperationtest_SRCS areaoperationtest.cpp areaprinter.cpp )
kde4_automoc(${areaoperationtest_SRCS})
kde4_add_executable(sublime-areaoperationtest NOGUI RUN_UNINSTALLED ${areaoperationtest_SRCS})
target_link_libraries(sublime-areaoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )
add_test(sublime-areaoperationtest ${EXECUTABLE_OUTPUT_PATH}/sublime-areaoperationtest)

set( areawalkertest_SRCS areawalkertest.cpp areaprinter.cpp )
kde4_automoc(${areawalkertest_SRCS})
kde4_add_executable(sublime-areawalkertest NOGUI RUN_UNINSTALLED ${areawalkertest_SRCS})
target_link_libraries(sublime-areawalkertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )
add_test(sublime-areawalkertest ${EXECUTABLE_OUTPUT_PATH}/sublime-areawalkertest)

set( viewtest_SRCS viewtest.cpp )
kde4_automoc(${viewtest_SRCS})
kde4_add_executable(sublime-viewtest NOGUI RUN_UNINSTALLED ${viewtest_SRCS})
target_link_libraries(sublime-viewtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )
add_test(sublime-viewtest ${EXECUTABLE_OUTPUT_PATH}/sublime-viewtest)

set( documenttest_SRCS documenttest.cpp )
kde4_automoc(${documenttest_SRCS})
kde4_add_executable(sublime-documenttest NOGUI RUN_UNINSTALLED ${documenttest_SRCS})
target_link_libraries(sublime-documenttest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )
add_test(sublime-documenttest ${EXECUTABLE_OUTPUT_PATH}/sublime-documenttest)

set( controllertest_SRCS controllertest.cpp )
kde4_automoc(${controllertest_SRCS})
kde4_add_executable(sublime-controllertest NOGUI RUN_UNINSTALLED ${controllertest_SRCS})
target_link_libraries(sublime-controllertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )
add_test(sublime-controllertest ${EXECUTABLE_OUTPUT_PATH}/sublime-controllertest)

endif(KDE4_BUILD_TESTS)
