set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${XtestPlugin_SOURCE_DIR}/qxqtest
        ${XtestPlugin_SOURCE_DIR}) # for kasserts.h

macro(kdev_qx_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(qxrunner-${test_EXEC} ${test_SRCS})
    target_link_libraries(qxrunner-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        qxrunner
        qxqtest)
endmacro(kdev_qx_add_test)

kdev_qx_add_test(ut-qtestxmlparser "qtestxmlparsertest.cpp")
kdev_qx_add_test(ut-testcase "qtestcasetest.cpp")
kdev_qx_add_test(ut-testcommand "qtestcommandtest.cpp")
kdev_qx_add_test(ut-testbase "qtestbasetest.cpp")
kdev_qx_add_test(ut-testsuite "qtestsuitetest.cpp")
kdev_qx_add_test(ut-testregister "qtestregistertest.cpp")
