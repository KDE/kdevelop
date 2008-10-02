add_subdirectory(tests)

#
# TODO install qxcppunit headers
#

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

include_directories(
    ${XtestPlugin_SOURCE_DIR}
    )

set(kdevcppunit_SRCS
    testcommand.cpp
    testcase.cpp
    testsuite.cpp
    testroot.cpp
    cppunitviewdata.cpp
    outputparser.cpp)

qt4_add_resources(kdevcppunit_SRCS
    resources/qxcppunit.qrc)

qt4_automoc( ${kdevcppunit_SRCS} )
kde4_add_library(kdevcppunit SHARED ${kdevcppunit_SRCS})
target_link_libraries(kdevcppunit
    ${KDEVPLATFORM_VERITAS_LIBRARY}
    ${CPPUNIT_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY})
install(TARGETS kdevcppunit ${INSTALL_TARGETS_DEFAULT_ARGS} )

#########################################

set(kdevcppunitview_SRCS cppunitplugin.cpp)
qt4_automoc(${kdevcppunitview_SRCS})
kde4_add_plugin(kdevcppunitview ${kdevcppunitview_SRCS})
target_link_libraries(kdevcppunitview
    kdevcppunit
    ${KDEVPLATFORM_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY})
install(TARGETS kdevcppunitview DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevcppunit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcppunit.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcppunit )
install( FILES cppunitwrapper.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/veritas/ )
