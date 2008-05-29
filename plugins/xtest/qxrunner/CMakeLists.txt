add_subdirectory(tests)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(TestBuild on)
if(TestBuild)
# expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${XtestPlugin_BINARY_DIR})

########### next target ###############

set(qxrunner_SRCS
    proxymodelcommon.cpp
    qxrunner_global.cpp
    resultsmodel.cpp
    resultsproxymodel.cpp
    resultsviewcontroller.cpp
    runner.cpp
    runneritem.cpp
    runnermodel.cpp
    runnermodelthread.cpp
    runnerproxymodel.cpp
    runnerviewcontroller.cpp
    runnerwindow.cpp
    statuswidget.cpp
    stoppingdialog.cpp
    utils.cpp
    viewcontrollercommon.cpp)

kde4_add_ui_files(qxrunner_SRCS
    runnerwindow.ui
    statuswidget.ui
    stoppingdialog.ui)

qt4_add_resources(qxrunner_SRCS
    resources/qxrunner.qrc)

kde4_add_library(qxrunner SHARED ${qxrunner_SRCS})
target_link_libraries(qxrunner
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS})
qt4_automoc(${qxrunner_SRCS})
install(TARGETS qxrunner ${INSTALL_TARGETS_DEFAULT_ARGS})
