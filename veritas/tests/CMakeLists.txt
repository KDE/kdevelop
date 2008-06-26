set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${CMAKE_SOURCE_DIR}/veritas
        ${CMAKE_SOURCE_DIR}/veritas/tests
        ${CMAKE_SOURCE_DIR}/veritas/interfaces
        ${CMAKE_BINARY_DIR}/veritas)

macro(kdev_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(veritas-${test_EXEC} ${test_SRCS} )
    target_link_libraries(veritas-${test_EXEC}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        kdevplatformveritas)
endmacro(kdev_add_test)

macro(kdev_add_test2 test_EXEC test_SRCS more_SRCS)
    qt4_automoc( ${test_SRCS} ${more_SRCS} )
    kde4_add_unit_test(veritas-${test_EXEC} ${test_SRCS} ${more_SRCS})
    target_link_libraries(veritas-${test_EXEC}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        kdevplatformveritas)
endmacro(kdev_add_test2)

kdev_add_test(ut-test testtest.cpp)
kdev_add_test2(ut-resultsmodel ${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp resultsmodeltest.cpp)
kdev_add_test2(ut-runnermodel ${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp runnermodeltest.cpp)
kdev_add_test(ut-proxymodelcommon proxymodelcommontest.cpp)
kdev_add_test(ut-resultsproxymodel resultsproxymodeltest.cpp)
kdev_add_test(ut-runnerproxymodel runnerproxymodeltest.cpp)
kdev_add_test(it-runnerwindow runnerwindowtest.cpp)

# kdev_qx_add_test(viewcontrollercommon-tdd "viewcontrollercommontest.cpp")
# kdev_qx_add_test(runnerviewcontroller-tdd "runnerviewcontrollertest.cpp")
# kdev_qx_add_test(resultviewcontroller-tdd "resultviewcontrollertest.cpp")
# kdev_qx_add_test(utils-tdd "utilstest.cpp")
