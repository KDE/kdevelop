add_subdirectory(tests)

#
# TODO install qxcheck headers
#

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

set(TestBuild on)
if(TestBuild)
# expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${XtestPlugin_SOURCE_DIR}
    ${XtestPlugin_BINARY_DIR}/qxrunner)

set(qxcheck_SRCS
    checkmodel.cpp
    checkregister.cpp
    checkoutputparser.cpp
    testrunner.cpp
    testbase.cpp
    testcase.cpp
    testsuite.cpp
    testcommand.cpp
    testresult.cpp)

qt4_automoc( ${qxcheck_SRCS} )
kde4_add_library(qxcheck SHARED ${qxcheck_SRCS})
target_link_libraries(qxcheck qxrunner ${CHECK_LIBRARY})
install(TARGETS qxcheck ${INSTALL_TARGETS_DEFAULT_ARGS} )
