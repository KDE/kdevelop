add_subdirectory(tests)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

########### next target ###############

set(veritas_SRCS
    test.cpp
    testresult.cpp
    testexecutor.cpp
    testrunnertoolview.cpp
    utils.cpp
    selectionstore.cpp
    mvc/resultsmodel.cpp
    mvc/resultsproxymodel.cpp
    mvc/runnermodel.cpp
    mvc/runnerproxymodel.cpp
    mvc/runnerwindow.cpp
    mvc/stoppingdialog.cpp
    mvc/selectionmanager.cpp
    mvc/selectiontoggle.cpp
    mvc/verbosemanager.cpp
    mvc/verbosetoggle.cpp
    mvc/overlaymanager.cpp
    mvc/overlaytoggle.cpp
    mvc/resultswidget.cpp
    tests/runnertesthelper.cpp)
    #${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp)

kde4_add_ui_files(veritas_SRCS
    mvc/runnerwindow.ui
    mvc/stoppingdialog.ui)

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
    testrunnertoolview.h
    tests/runnertesthelper.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/veritas )
