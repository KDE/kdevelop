project(QxRunnerPlugin)
find_package(KDE4 REQUIRED)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
include_directories( ${KDE4_INCLUDES}
                     ${PROJECT_BINARY_DIR} )
add_subdirectory(tests)

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
    viewcontrollercommon.cpp
    tests/qxtsignalwaiter.cpp)

kde4_add_ui_files( qxrunner_SRCS
    runnerwindow.ui
    statuswidget.ui
    stoppingdialog.ui)

qt4_add_resources(qxrunner_SRCS ${PROJECT_SOURCE_DIR}/resources/qxrunner.qrc )
qt4_automoc( ${qxrunner_SRCS} )

kde4_add_library(qxrunner STATIC ${qxrunner_SRCS})
# constructing a shared library compiles and links, using it fails horribly though. havent found out why .. yet
target_link_libraries(qxrunner
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS})
set_target_properties(qxrunner PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install(TARGETS qxrunner DESTINATION ${LIB_INSTALL_DIR} )

########### next target ###############

# TODO install qxcppunit headers to include when building the tests
