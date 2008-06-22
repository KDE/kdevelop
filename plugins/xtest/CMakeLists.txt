project(XtestPlugin)

#set(CMAKE_BUILD_TYPE debug)

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
macro_log_feature(CPPUNIT_FOUND "CppUnit" "A C++ unit testing framework" "http://cppunit.sourceforge.net" FALSE "" "Needed for building qxcppunit")

if (CPPUNIT_FOUND)
    add_subdirectory(qxcppunit)
endif (CPPUNIT_FOUND)

# TODO write a proper FindCheck.cmake

find_path(CHECK_INCLUDE_DIR
    check.h
    /usr/include /usr/local/include)

find_library(CHECK_LIBRARY
    NAMES libcheck.so libcheck.so.0 libcheck.so.0.0.0
    PATH /usr/lib /usr/local/lib)

if (CHECK_INCLUDE_DIR AND CHECK_LIBRARY)
    set(CHECK_FOUND on)
endif (CHECK_INCLUDE_DIR AND CHECK_LIBRARY)
macro_log_feature(CHECK_FOUND "Check" "A unit testing framework for C" "http://check.sourceforge.net/" FALSE "" "Needed for building qxcheck")

if (CHECK_FOUND)
    add_subdirectory(qxcheck)
endif (CHECK_FOUND)


include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/shell
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces)

########### next target #################

set(xtestconfig_SRCS xtestconfigwidget.cpp)
kde4_add_kcfg_files(xtestconfig_SRCS xtestconfig.kcfgc)
kde4_add_ui_files(xtestconfig_SRCS xtestconfigwidget.ui)
kde4_add_plugin(kcm_kdevxtest_config ${xtestconfig_SRCS} )
target_link_libraries(kcm_kdevxtest_config
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY})

install(TARGETS kcm_kdevxtest_config DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

set(kdevxtest_SRCS xtestplugin.cpp)
kde4_add_plugin(kdevxtest ${kdevxtest_SRCS})

target_link_libraries(kdevxtest
    ${QT_QTDESIGNER_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    kdevplatformshell
    kdevplatforminterfaces
    kdevplatformoutputview
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    qxqtest)

if (CPPUNIT_FOUND)
    target_link_libraries( kdevxtest
                           qxcppunit)
endif (CPPUNIT_FOUND)

if (CHECK_FOUND)
    target_link_libraries( kdevxtest
                           qxcheck)
endif (CHECK_FOUND)

install(TARGETS kdevxtest DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevxtest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevxtest.rc DESTINATION ${DATA_INSTALL_DIR}/kdevxtest )
install( FILES kcm_kdevxtest_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
