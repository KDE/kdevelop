project(unittestskde)

enable_testing()
set(KDE4_BUILD_TESTS ON)

find_package(KDE4 REQUIRED)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )

kde4_add_unit_test(cmake-test-unittestskde test.cpp)
target_link_libraries(cmake-test-unittestskde ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})
