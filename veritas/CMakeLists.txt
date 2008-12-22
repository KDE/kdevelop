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
    testexecutableinfo.cpp
    ctestfileparser.cpp
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
    internal/verbosetoggle.cpp
    internal/overlaymanager.cpp
    internal/overlaytoggle.cpp
    internal/resultswidget.cpp
    internal/itestframework_p.cpp
    internal/tosourcetoggle.cpp
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
set_target_properties(kdevplatformveritas PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
target_link_libraries(kdevplatformveritas
    kdevplatforminterfaces
    sublime
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS})
target_link_libraries(kdevplatformveritas LINK_INTERFACE_LIBRARIES 
        ${KDE4_KDECORE_LIBS} 
        ${KDE4_KTEXTEDITOR_LIBS} 
        ${KDE4_KDEUI_LIBS} 
        ${QT_QTTEST_LIBRARY})
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
    testexecutableinfo.h
    ctestfileparser.h
    tests/runnertesthelper.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/veritas COMPONENT Devel )
