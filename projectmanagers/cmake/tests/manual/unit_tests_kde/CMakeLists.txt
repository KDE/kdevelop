project(unittestskde)

enable_testing()
set(KDE4_BUILD_TESTS ON)

find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
find_package(KDE4 REQUIRED)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )

set(unittestskde_test_SRCS test.cpp)
kde4_add_unit_test(cmake-test-unittestskde ${unittestskde_test_SRCS})
target_link_libraries(cmake-test-unittestskde ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

kde4_add_unit_test(unittestskde-nonstd-location ${unittestskde_test_SRCS})
target_link_libraries(unittestskde-nonstd-location ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})
set_target_properties(unittestskde-nonstd-location PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
