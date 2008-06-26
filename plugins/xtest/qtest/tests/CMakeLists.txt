add_subdirectory(suite1)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${XtestPlugin_SOURCE_DIR}/qtest
        ${XtestPlugin_SOURCE_DIR}  # for kasserts.h
        ${KDEVPLATFORM_INCLUDE_DIR}/veritas
        ${KDEVPLATFORM_INCLUDE_DIR}/veritas/interfaces)

macro(kdev_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(qtest-${test_EXEC} ${test_SRCS})
    target_link_libraries(qtest-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevqtest)
endmacro(kdev_add_test)

kdev_add_test(ut-qtestoutputparser "qtestoutputparsertest.cpp")
kdev_add_test(ut-qtestcase "qtestcasetest.cpp")
kdev_add_test(ut-qtestcommand "qtestcommandtest.cpp")
kdev_add_test(ut-qtestsuite "qtestsuitetest.cpp")
kdev_add_test(ut-qtestregister "qtestregistertest.cpp")
kdev_add_test(it-qtestrunnertest qtestrunnertest.cpp)

add_executable(suite1/fakeqtest1 fakeqtest1_2cmds.cpp)
target_link_libraries(suite1/fakeqtest1 ${KDE4_KDECORE_LIBS})

add_executable(suite1/fakeqtest2 fakeqtest2_2cmds_fail.cpp)
target_link_libraries(suite1/fakeqtest2 ${KDE4_KDECORE_LIBS})

add_executable(suite1/fakeqtest3 fakeqtest3_slow.cpp)
target_link_libraries(suite1/fakeqtest3 ${KDE4_KDECORE_LIBS})
