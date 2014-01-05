set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( areaoperationtest_SRCS areaoperationtest.cpp areaprinter.cpp )
add_executable(sublime-areaoperationtest ${areaoperationtest_SRCS})
ecm_mark_as_test(sublime-areaoperationtest)
target_link_libraries(sublime-areaoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( areawalkertest_SRCS areawalkertest.cpp areaprinter.cpp )
add_executable(sublime-areawalkertest ${areawalkertest_SRCS})
ecm_mark_as_test(sublime-areawalkertest)
target_link_libraries(sublime-areawalkertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( viewtest_SRCS viewtest.cpp )
add_executable(sublime-viewtest ${viewtest_SRCS})
ecm_mark_as_test(sublime-viewtest)
target_link_libraries(sublime-viewtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( documenttest_SRCS documenttest.cpp )
add_executable(sublime-documenttest ${documenttest_SRCS})
ecm_mark_as_test(sublime-documenttest)
target_link_libraries(sublime-documenttest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( controllertest_SRCS controllertest.cpp )
add_executable(sublime-controllertest ${controllertest_SRCS})
ecm_mark_as_test(sublime-controllertest)
target_link_libraries(sublime-controllertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( viewactivationtest_SRCS viewactivationtest.cpp )
add_executable(sublime-viewactivationtest ${viewactivationtest_SRCS})
ecm_mark_as_test(sublime-viewactivationtest)
target_link_libraries(sublime-viewactivationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( aggregatemodeltest_SRCS aggregatemodeltest.cpp )
add_executable(sublime-aggregatemodeltest ${aggregatemodeltest_SRCS})
ecm_mark_as_test(sublime-aggregatemodeltest)
target_link_libraries(sublime-aggregatemodeltest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime kdevplatformtests )

set( toolviewtoolbartest_SRCS toolviewtoolbartest.cpp )
add_executable(sublime-toolviewtoolbartest ${toolviewtoolbartest_SRCS})
ecm_mark_as_test(sublime-toolviewtoolbartest)
target_link_libraries(sublime-toolviewtoolbartest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )

