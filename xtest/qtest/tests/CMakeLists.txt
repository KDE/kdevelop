
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${XtestPlugin_SOURCE_DIR}/qtest
        ${XtestPlugin_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

macro(kdev_add_test test_EXEC)
    set(test_SRCS ${ARGN})
    kde4_add_unit_test(qtest-${test_EXEC} ${test_SRCS})
    target_link_libraries(qtest-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevqtest)
endmacro(kdev_add_test)

kdev_add_test(unit-qtestoutputparser qtestoutputparsertest.cpp outputparsertestmain.cpp)
kdev_add_test(unit-qtestcase "qtestcasetest.cpp")
kdev_add_test(unit-qtestcommand "qtestcommandtest.cpp")
kdev_add_test(unit-qtestsuite "qtestsuitetest.cpp")
kdev_add_test(unit-qtestregister "qtestregistertest.cpp")
kdev_add_test(unit-qtestoutputmorpher "qtestoutputmorphertest.cpp")
kdev_add_test(sys-qtestrunnertest qtestrunnertest.cpp)
kdev_add_test(unit-suitebuilder suitebuildertest.cpp)
kdev_add_test(unit-casebuilder casebuildertest.cpp)
kdev_add_test(unit-newtestwizardtest newtestwizardtest.cpp)

add_subdirectory(suite1)

add_executable(qtestrunner standalonerunner.cpp)
qt4_automoc(standalonerunner.cpp)
target_link_libraries(qtestrunner
        ${QT_QTGUI_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevqtest)

add_executable(parserstresstest parserstresstest.cpp qtestoutputparsertest.cpp)
qt4_automoc( qtestoutputparsertest.cpp )
target_link_libraries(parserstresstest
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_VERITAS_LIBRARY}
    kdevqtest)

add_test(qtest-fuzz-validxml ${CMAKE_CURRENT_BINARY_DIR}/parserstresstest valid --quiet)
add_test(qtest-fuzz-garbagexml ${CMAKE_CURRENT_BINARY_DIR}/parserstresstest garbage --quiet)

macro(qtest_add_mem_test test_EXEC prefix)
    add_test(qtest-mem-${test_EXEC}
        ${CMAKE_CURRENT_SOURCE_DIR}/runMemcheck.py ${CMAKE_CURRENT_BINARY_DIR}/qtest-${prefix}-${test_EXEC} ${CMAKE_BINARY_DIR})
endmacro(qtest_add_mem_test)

qtest_add_mem_test(qtestoutputparser unit)
qtest_add_mem_test(qtestcase unit)
qtest_add_mem_test(qtestcommand unit)
qtest_add_mem_test(qtestsuite unit)
qtest_add_mem_test(qtestregister unit)
qtest_add_mem_test(qtestoutputmorpher unit)
qtest_add_mem_test(suitebuilder unit)
qtest_add_mem_test(casebuilder unit)
qtest_add_mem_test(qtestrunnertest sys)
