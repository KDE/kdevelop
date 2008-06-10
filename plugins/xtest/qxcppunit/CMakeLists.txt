#
# TODO install qxcppunit headers
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

set(qxcppunit_SRCS
    cppunititem.cpp
    cppunitmodel.cpp
    testrunner.cpp)

qt4_add_resources(qxcppunit_SRCS
    resources/qxcppunit.qrc)

qt4_automoc( ${qxcppunit_SRCS} )
kde4_add_library(qxcppunit SHARED ${qxcppunit_SRCS})
target_link_libraries(qxcppunit qxrunner ${CPPUNIT_LIBRARY})
install(TARGETS qxcppunit ${INSTALL_TARGETS_DEFAULT_ARGS} )
