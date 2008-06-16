add_subdirectory(suite1)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${XtestPlugin_SOURCE_DIR}/qxqtest
        ${XtestPlugin_SOURCE_DIR}  # for kasserts.h
        ${XtestPlugin_BINARY_DIR}/qxrunner)

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

kdev_qx_add_test(ut-qtestoutputparser "qtestoutputparsertest.cpp")
kdev_qx_add_test(ut-qtestcase "qtestcasetest.cpp")
kdev_qx_add_test(ut-qtestcommand "qtestcommandtest.cpp")
kdev_qx_add_test(ut-qtestbase "qtestbasetest.cpp")
kdev_qx_add_test(ut-qtestsuite "qtestsuitetest.cpp")
kdev_qx_add_test(ut-qtestregister "qtestregistertest.cpp")
kdev_qx_add_test(it-qtestrunnertest "qtestrunnertest.cpp")

add_executable(suite1/fakeqtest1 fakeqtest1_2cmds.cpp)
target_link_libraries(suite1/fakeqtest1 ${KDE4_KDECORE_LIBS})

add_executable(suite1/fakeqtest2 fakeqtest2_2cmds_fail.cpp)
target_link_libraries(suite1/fakeqtest2 ${KDE4_KDECORE_LIBS})

add_executable(suite1/fakeqtest3 fakeqtest3_slow.cpp)
target_link_libraries(suite1/fakeqtest3 ${KDE4_KDECORE_LIBS})
