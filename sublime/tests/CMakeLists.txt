set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( areaoperationtest_SRCS areaoperationtest.cpp areaprinter.cpp )
kde4_add_unit_test(sublime-areaoperationtest ${areaoperationtest_SRCS})
target_link_libraries(sublime-areaoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( areawalkertest_SRCS areawalkertest.cpp areaprinter.cpp )
kde4_add_unit_test(sublime-areawalkertest ${areawalkertest_SRCS})
target_link_libraries(sublime-areawalkertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( viewtest_SRCS viewtest.cpp )
kde4_add_unit_test(sublime-viewtest ${viewtest_SRCS})
target_link_libraries(sublime-viewtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( documenttest_SRCS documenttest.cpp )
kde4_add_unit_test(sublime-documenttest ${documenttest_SRCS})
target_link_libraries(sublime-documenttest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( controllertest_SRCS controllertest.cpp )
kde4_add_unit_test(sublime-controllertest ${controllertest_SRCS})
target_link_libraries(sublime-controllertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( viewactivationtest_SRCS viewactivationtest.cpp )
kde4_add_unit_test(sublime-viewactivationtest ${viewactivationtest_SRCS})
target_link_libraries(sublime-viewactivationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )


set( aggregatemodeltest_SRCS aggregatemodeltest.cpp )
kde4_add_unit_test(sublime-aggregatemodeltest ${aggregatemodeltest_SRCS})
target_link_libraries(sublime-aggregatemodeltest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime kdevplatformtests )

set( toolviewtoolbartest_SRCS toolviewtoolbartest.cpp )
kde4_add_unit_test(sublime-toolviewtoolbartest ${toolviewtoolbartest_SRCS})
target_link_libraries(sublime-toolviewtoolbartest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime )

