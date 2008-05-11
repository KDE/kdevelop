set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${XtestPlugin_SOURCE_DIR}/qxrunner ${XtestPlugin_BINARY_DIR})

macro(kdev_qx_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(qxrunner-${test_EXEC} ${test_SRCS})
    target_link_libraries(qxrunner-${test_EXEC}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        qxrunner)
endmacro(kdev_qx_add_test)

kdev_qx_add_test(ut-runneritem "runneritemtest.cpp")
kdev_qx_add_test(ut-resultsmodel "resultsmodeltest.cpp")
kdev_qx_add_test(ut-runnermodel "runnermodeltest.cpp")
kdev_qx_add_test(ut-proxymodelcommon "proxymodelcommontest.cpp")
kdev_qx_add_test(ut-resultsproxymodel "resultsproxymodeltest.cpp")
kdev_qx_add_test(ut-runnerproxymodel "runnerproxymodeltest.cpp")
kdev_qx_add_test(it-runnerwindow "runnerwindowtest.cpp")

# kdev_qx_add_test(viewcontrollercommon-tdd "viewcontrollercommontest.cpp")
# kdev_qx_add_test(runnerviewcontroller-tdd "runnerviewcontrollertest.cpp")
# kdev_qx_add_test(resultviewcontroller-tdd "resultviewcontrollertest.cpp")
# kdev_qx_add_test(utils-tdd "utilstest.cpp")
