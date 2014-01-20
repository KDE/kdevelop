set( areaoperationtest_SRCS areaoperationtest.cpp areaprinter.cpp )
add_executable(sublime-areaoperationtest ${areaoperationtest_SRCS})
add_test(sublime-areaoperationtest sublime-areaoperationtest)
ecm_mark_as_test(sublime-areaoperationtest)
target_link_libraries(sublime-areaoperationtest Qt5::Test KF5::Parts sublime )


set( areawalkertest_SRCS areawalkertest.cpp areaprinter.cpp )
add_executable(sublime-areawalkertest ${areawalkertest_SRCS})
add_test(sublime-areawalkertest sublime-areawalkertest)
ecm_mark_as_test(sublime-areawalkertest)
target_link_libraries(sublime-areawalkertest Qt5::Test KF5::Parts sublime )


set( viewtest_SRCS viewtest.cpp )
add_executable(sublime-viewtest ${viewtest_SRCS})
add_test(sublime-viewtest sublime-viewtest)
ecm_mark_as_test(sublime-viewtest)
target_link_libraries(sublime-viewtest Qt5::Test KF5::Parts sublime )


set( documenttest_SRCS documenttest.cpp )
add_executable(sublime-documenttest ${documenttest_SRCS})
add_test(sublime-documenttest sublime-documenttest)
ecm_mark_as_test(sublime-documenttest)
target_link_libraries(sublime-documenttest Qt5::Test KF5::Parts sublime )


set( controllertest_SRCS controllertest.cpp )
add_executable(sublime-controllertest ${controllertest_SRCS})
add_test(sublime-controllertest sublime-controllertest)
ecm_mark_as_test(sublime-controllertest)
target_link_libraries(sublime-controllertest Qt5::Test KF5::Parts sublime )


set( viewactivationtest_SRCS viewactivationtest.cpp )
add_executable(sublime-viewactivationtest ${viewactivationtest_SRCS})
add_test(sublime-viewactivationtest sublime-viewactivationtest)
ecm_mark_as_test(sublime-viewactivationtest)
target_link_libraries(sublime-viewactivationtest Qt5::Test KF5::Parts sublime )


set( aggregatemodeltest_SRCS aggregatemodeltest.cpp )
add_executable(sublime-aggregatemodeltest ${aggregatemodeltest_SRCS})
add_test(sublime-aggregatemodeltest sublime-aggregatemodeltest)
ecm_mark_as_test(sublime-aggregatemodeltest)
target_link_libraries(sublime-aggregatemodeltest Qt5::Test KF5::Parts sublime kdevplatformtests )

set( toolviewtoolbartest_SRCS toolviewtoolbartest.cpp )
add_executable(sublime-toolviewtoolbartest ${toolviewtoolbartest_SRCS})
add_test(sublime-toolviewtoolbartest sublime-toolviewtoolbartest)
ecm_mark_as_test(sublime-toolviewtoolbartest)
target_link_libraries(sublime-toolviewtoolbartest Qt5::Test KF5::Parts sublime )

