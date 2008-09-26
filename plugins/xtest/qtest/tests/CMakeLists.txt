
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${XtestPlugin_SOURCE_DIR}/qtest
        ${XtestPlugin_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

macro(kdev_add_test test_EXEC)
    set(test_SRCS ${ARGN})
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(qtest-${test_EXEC} ${test_SRCS})
    target_link_libraries(qtest-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevqtest)
endmacro(kdev_add_test)

kdev_add_test(ut-qtestoutputparser qtestoutputparsertest.cpp outputparsertestmain.cpp)
kdev_add_test(ut-qtestcase "qtestcasetest.cpp")
kdev_add_test(ut-qtestcommand "qtestcommandtest.cpp")
kdev_add_test(ut-qtestsuite "qtestsuitetest.cpp")
kdev_add_test(ut-qtestregister "qtestregistertest.cpp")
kdev_add_test(ut-qtestoutputmorpher "qtestoutputmorphertest.cpp")
kdev_add_test(it-qtestrunnertest qtestrunnertest.cpp)
kdev_add_test(ut-suitebuilder suitebuildertest.cpp)
kdev_add_test(ut-casebuilder casebuildertest.cpp)

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

macro(qtest_add_mem_test test_EXEC)
    add_test(qtest-mem-${test_EXEC}
        ${CMAKE_CURRENT_SOURCE_DIR}/runMemcheck.py ${CMAKE_CURRENT_BINARY_DIR}/qtest-ut-${test_EXEC} ${CMAKE_BINARY_DIR})
endmacro(qtest_add_mem_test)

qtest_add_mem_test(qtestoutputparser)
qtest_add_mem_test(qtestcase)
qtest_add_mem_test(qtestcommand)
qtest_add_mem_test(qtestsuite)
qtest_add_mem_test(qtestregister)
qtest_add_mem_test(qtestoutputmorpher)
qtest_add_mem_test(suitebuilder)
qtest_add_mem_test(casebuilder)
