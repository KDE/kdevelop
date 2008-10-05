set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${kdevplatform_SOURCE_DIR}/interfaces)

macro(kdev_add_test test_EXEC)
    set(test_SRCS ${ARGN})
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(veritas-${test_EXEC} ${test_SRCS})
    target_link_libraries(veritas-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevqtest)
endmacro(kdev_add_test)

kdev_add_test(ut-test testtest.cpp)
kdev_add_test(ut-resultsmodel ${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp resultsmodeltest.cpp)
kdev_add_test(ut-runnermodel ${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp runnermodeltest.cpp)
kdev_add_test(ut-resultsproxymodel resultsproxymodeltest.cpp)
kdev_add_test(ut-testexecutor testexecutortest.cpp)
kdev_add_test(it-runnerwindow runnerwindowtest.cpp)
kdev_add_test(ut-selectionstoretest selectionstoretest.cpp)
kdev_add_test(ut-treetraverse treetraversetest.cpp)
kdev_add_test(ut-resultswidget resultswidgettest.cpp)
