include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/sublime
    )

set( areaoperationtest_SRCS areaoperationtest.cpp areaprinter.cpp )
kde4_add_unit_test(sublime-areaoperationtest TESTNAME sublime-sublime-areaoperationtest  ${areaoperationtest_SRCS})
target_link_libraries(sublime-areaoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )


set( areawalkertest_SRCS areawalkertest.cpp areaprinter.cpp )
kde4_add_unit_test(sublime-areawalkertest TESTNAME sublime-sublime-areawalkertest  ${areawalkertest_SRCS})
target_link_libraries(sublime-areawalkertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )


set( viewtest_SRCS viewtest.cpp )
kde4_add_unit_test(sublime-viewtest TESTNAME sublime-sublime-viewtest  ${viewtest_SRCS})
target_link_libraries(sublime-viewtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )


set( documenttest_SRCS documenttest.cpp )
kde4_add_unit_test(sublime-documenttest TESTNAME sublime-sublime-documenttest  ${documenttest_SRCS})
target_link_libraries(sublime-documenttest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )


set( controllertest_SRCS controllertest.cpp )
kde4_add_unit_test(sublime-controllertest TESTNAME sublime-sublime-controllertest  ${controllertest_SRCS})
target_link_libraries(sublime-controllertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )


set( viewactivationtest_SRCS viewactivationtest.cpp )
kde4_add_unit_test(sublime-viewactivationtest TESTNAME sublime-sublime-viewactivationtest  ${viewactivationtest_SRCS})
target_link_libraries(sublime-viewactivationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )


set( aggregatemodeltest_SRCS aggregatemodeltest.cpp ${CMAKE_SOURCE_DIR}/test/common/modeltest.cpp )
kde4_add_unit_test(sublime-aggregatemodeltest TESTNAME sublime-sublime-aggregatemodeltest  ${aggregatemodeltest_SRCS})
target_link_libraries(sublime-aggregatemodeltest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} sublime )

