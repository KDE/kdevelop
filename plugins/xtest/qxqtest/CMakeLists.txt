add_subdirectory(tests)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(TestBuild on)
if(TestBuild)
    # expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${XtestPlugin_SOURCE_DIR} # for <qxrunner/xxx> includes
    ${XtestPlugin_BINARY_DIR}/qxrunner) #for ui_runnerwindow.h and friends

set(qxqtest_SRCS
    qtestresult.cpp
    qtestoutputparser.cpp
    qtestcase.cpp
    qtestcommand.cpp
    qtestbase.cpp
    qtestsuite.cpp
    qtestregister.cpp
    qtestitem.cpp
    qtestmodel.cpp
    testrunner.cpp)

qt4_automoc(q${xqtest_SRCS})
kde4_add_library(qxqtest SHARED ${qxqtest_SRCS})
target_link_libraries(qxqtest ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} qxrunner)
install(TARGETS qxqtest ${INSTALL_TARGETS_DEFAULT_ARGS} )

