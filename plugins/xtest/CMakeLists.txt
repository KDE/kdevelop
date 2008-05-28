
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qxrunner)
add_subdirectory(qxqtest)
add_subdirectory(qxcppunit) # need to write a FindCppUnit.cmake

project(XtestPlugin)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(TestBuild on)
if(TestBuild)
    # expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${PROJECT_BINARY_DIR} # for ui*.h
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/qxrunner
    ${PROJECT_SOURCE_DIR}/qxcppunit
)

########### next target ###############

set(qxrunner_SRCS
    qxrunner/proxymodelcommon.cpp
    qxrunner/qxrunner_global.cpp
    qxrunner/resultsmodel.cpp
    qxrunner/resultsproxymodel.cpp
    qxrunner/resultsviewcontroller.cpp
    qxrunner/runner.cpp
    qxrunner/runneritem.cpp
    qxrunner/runnermodel.cpp
    qxrunner/runnermodelthread.cpp
    qxrunner/runnerproxymodel.cpp
    qxrunner/runnerviewcontroller.cpp
    qxrunner/runnerwindow.cpp
    qxrunner/statuswidget.cpp
    qxrunner/stoppingdialog.cpp
    qxrunner/utils.cpp
    qxrunner/viewcontrollercommon.cpp
)

kde4_add_ui_files(qxrunner_SRCS
    qxrunner/runnerwindow.ui
    qxrunner/statuswidget.ui
    qxrunner/stoppingdialog.ui)

qt4_add_resources(qxrunner_SRCS
    ${PROJECT_SOURCE_DIR}/qxrunner/resources/qxrunner.qrc)

kde4_add_library(qxrunner SHARED ${qxrunner_SRCS})
target_link_libraries(qxrunner
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS})

qt4_automoc(${qxrunner_SRCS})
install(TARGETS qxrunner DESTINATION ${LIB_INSTALL_DIR})

# ########### next target ###############
#
# TODO install qxcppunit headers to include when building the tests
#
########### next target ###############

find_library(CPPUNIT_LIBRARY
    NAMES cppunit-1.12 libcppunit-1.12 libcppunit cppunit
    PATH /usr/lib)

set(qxcppunit_SRCS
    qxcppunit/cppunititem.cpp
    qxcppunit/cppunitmodel.cpp
    qxcppunit/testrunner.cpp)

qt4_add_resources(qxcppunit_SRCS
    ${PROJECT_SOURCE_DIR}/qxcppunit/resources/qxcppunit.qrc)

qt4_automoc( ${qxcppunit_SRCS} )
kde4_add_library(qxcppunit SHARED ${qxcppunit_SRCS})
target_link_libraries(qxcppunit qxrunner ${CPPUNIT_LIBRARY})
install(TARGETS qxcppunit DESTINATION ${LIB_INSTALL_DIR})

########## next target ###############

set(qxqtest_SRCS
    qxqtest/qtestresult.cpp
    qxqtest/qtestxmlparser.cpp
    qxqtest/qtestcase.cpp
    qxqtest/qtestcommand.cpp
    qxqtest/qtestbase.cpp
    qxqtest/qtestsuite.cpp
    qxqtest/qtestregister.cpp
    qxqtest/qtestitem.cpp
    qxqtest/qtestmodel.cpp
    qxqtest/testrunner.cpp)

qt4_automoc(q${xqtest_SRCS})
kde4_add_library(qxqtest SHARED ${qxqtest_SRCS})
target_link_libraries(qxqtest ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} qxrunner)
install(TARGETS qxqtest DESTINATION ${LIB_INSTALL_DIR})

########## next target ###############

set(kdevxtest_SRCS xtestplugin.cpp)
kde4_add_plugin(kdevxtest ${kdevxtest_SRCS})

target_link_libraries(kdevxtest
    ${QT_QTDESIGNER_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces
    kdevplatformoutputview
    ${KDE4_THREADWEAVER_LIBRARIES}
    qxcppunit
    qxqtest
    )

install(TARGETS kdevxtest DESTINATION ${PLUGIN_INSTALL_DIR})

# ########### install files ###############

install( FILES kdevxtest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevxtest.rc DESTINATION ${DATA_INSTALL_DIR}/kdevxtest )
