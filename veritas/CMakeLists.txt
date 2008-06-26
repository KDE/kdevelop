add_subdirectory(tests)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

set(TestBuild on)
if(TestBuild)
# expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}/tests/common
    ${CMAKE_SOURCE_DIR}/veritas/interfaces )

########### next target ###############

set(veritas_SRCS
    proxymodelcommon.cpp
    resultsmodel.cpp
    resultsproxymodel.cpp
    resultsviewcontroller.cpp
    test.cpp
    testcommand.cpp
    testcase.cpp
    runnermodel.cpp
    runnermodelthread.cpp
    runnerproxymodel.cpp
    runnerviewcontroller.cpp
    runnerwindow.cpp
    statuswidget.cpp
    stoppingdialog.cpp
    utils.cpp
    viewcontrollercommon.cpp
    selectionmanager.cpp
    selectiontoggle.cpp
    testresult.cpp
    interfaces/itest.cpp)
#${CMAKE_SOURCE_DIR}/tests/common/modeltest.cpp)

kde4_add_ui_files(veritas_SRCS
    runnerwindow.ui
    statuswidget.ui
    stoppingdialog.ui)

qt4_add_resources(veritas_SRCS
    resources/qxrunner.qrc)

kde4_add_library(kdevplatformveritas SHARED ${veritas_SRCS})
target_link_libraries(kdevplatformveritas
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS})
qt4_automoc(${veritas_SRCS})
install(TARGETS kdevplatformveritas ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
    runnermodel.h
    test.h
    testcommand.h
    testcase.h
    register.h
    register.hpp
    runnerwindow.h
    veritasexport.h
    testresult.h
    ${CMAKE_BINARY_DIR}/veritas/ui_runnerwindow.h
    ${CMAKE_BINARY_DIR}/veritas/ui_statuswidget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/veritas )

install(FILES
    interfaces/itest.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/veritas/interfaces )
