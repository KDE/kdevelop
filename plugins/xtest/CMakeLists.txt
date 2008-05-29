project(XtestPlugin)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qxrunner)
add_subdirectory(qxqtest)

# TODO write a proper FindCppUnit.cmake

find_path(CPPUNIT_INCLUDE_DIR 
    TestSuite.h
    /usr/include/cppunit /usr/local/include/cppunit)

find_library(CPPUNIT_LIBRARY
    NAMES cppunit-1.12 libcppunit-1.12 libcppunit cppunit
    PATH /usr/lib /usr/local/lib)

if (CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)
    set(CPPUNIT_FOUND on)
endif (CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)

if (CPPUNIT_FOUND)
    add_subdirectory(qxcppunit)
else (CPPUNIT_FOUND)
    message(WARNING: CppUnit not found, wont compile qxcppunit)
endif (CPPUNIT_FOUND)

########### next target ###############

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR})

set(kdevxtest_SRCS xtestplugin.cpp)
kde4_add_plugin(kdevxtest ${kdevxtest_SRCS})

target_link_libraries(kdevxtest
    ${QT_QTDESIGNER_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces
    kdevplatformoutputview
    ${KDE4_THREADWEAVER_LIBRARIES}
    qxcppunit
    qxqtest)

install(TARGETS kdevxtest DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevxtest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevxtest.rc DESTINATION ${DATA_INSTALL_DIR}/kdevxtest )
