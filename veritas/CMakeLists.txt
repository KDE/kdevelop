add_subdirectory(tests)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

########### next target ###############

set(veritas_SRCS
    test.cpp
    testresult.cpp
    internal/testexecutor.cpp
    itestrunner.cpp
    itestframework.cpp
    internal/utils.cpp
    internal/selectionstore.cpp
    testtoolviewfactory.cpp
    internal/toolviewdata.cpp
    internal/resultsmodel.cpp
    internal/resultsproxymodel.cpp
    internal/runnermodel.cpp
    internal/runnerproxymodel.cpp
    internal/runnerwindow.cpp
    internal/selectionmanager.cpp
    internal/selectiontoggle.cpp
    internal/verbosemanager.cpp
    internal/verbosetoggle.cpp
    internal/overlaymanager.cpp
    internal/overlaytoggle.cpp
    internal/resultswidget.cpp
    internal/itestframework_p.cpp
    internal/test_p.cpp
    tests/runnertesthelper.cpp # Actual tests of concrete frameworks use these.
    tests/testutils.cpp)       # eg QTestRunnerTest, CppUnitRunnerTest
                               # Before ui_* and other internals were installed
                               # to make this possible, which is bad.
    #${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp)

kde4_add_ui_files(veritas_SRCS
    internal/runnerwindow.ui)

qt4_add_resources(veritas_SRCS
    resources/qxrunner.qrc)

kde4_add_library(kdevplatformveritas SHARED ${veritas_SRCS})
target_link_libraries(kdevplatformveritas
    kdevplatforminterfaces
    sublime
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS})
qt4_automoc(${veritas_SRCS})
install(TARGETS kdevplatformveritas ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
    test.h
    veritasexport.h
    testresult.h
    itestrunner.h
    itestframework.h
    testtoolviewfactory.h
    tests/runnertesthelper.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/veritas )
