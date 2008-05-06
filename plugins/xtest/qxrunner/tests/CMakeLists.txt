set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

macro(kdev_qx_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(qxrunner-${test_EXEC} ${test_SRCS})
    target_link_libraries(qxrunner-${test_EXEC}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${CMAKE_BINARY_DIR}/lib/libqxrunner.a)
endmacro(kdev_qx_add_test)

kdev_qx_add_test(runneritem "runneritemtest.cpp")
kdev_qx_add_test(resultsmodel "resultsmodeltest.cpp")
kdev_qx_add_test(runnermodel "runnermodeltest.cpp")
kdev_qx_add_test(proxymodelcommon "proxymodelcommontest.cpp")
kdev_qx_add_test(resultsproxymodel "resultsproxymodeltest.cpp")
kdev_qx_add_test(runnerproxymodel "runnerproxymodeltest.cpp")
kdev_qx_add_test(runnerwindow "runnerwindowtest.cpp")
# kdev_qx_add_test(viewcontrollercommon-tdd "viewcontrollercommontest.cpp")
# kdev_qx_add_test(runnerviewcontroller-tdd "runnerviewcontrollertest.cpp")
# kdev_qx_add_test(resultviewcontroller-tdd "resultviewcontrollertest.cpp")
# kdev_qx_add_test(utils-tdd "utilstest.cpp")
