# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/shell
    )

set(test_documentcontroller_SRCS test_documentcontroller.cpp)
add_executable(test_documentcontroller ${test_documentcontroller_SRCS})
add_test(test_documentcontroller test_documentcontroller)
ecm_mark_as_test(test_documentcontroller)
target_link_libraries( test_documentcontroller Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES})

set(test_uicontroller_SRCS test_uicontroller.cpp)
add_executable(test_uicontroller ${test_uicontroller_SRCS})
add_test(test_uicontroller test_uicontroller)
ecm_mark_as_test(test_uicontroller)
target_link_libraries( test_uicontroller Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES})

set( test_shellbuddy_SRCS test_shellbuddy.cpp )
add_executable(test_shellbuddy ${test_shellbuddy_SRCS})
add_test(test_shellbuddy test_shellbuddy)
ecm_mark_as_test(test_shellbuddy)
target_link_libraries(test_shellbuddy Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime )

set( test_shelldocumentoperation_SRCS test_shelldocumentoperation.cpp )
add_executable(test_shelldocumentoperation ${test_shelldocumentoperation_SRCS})
add_test(test_shelldocumentoperation test_shelldocumentoperation)
ecm_mark_as_test(test_shelldocumentoperation)
target_link_libraries(test_shelldocumentoperation Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime )

## This test needs fixing:
# creating a factory is not enough to inject a new project plugin, we also need a KPluginInfo
set( projectcontrollertest_SRCS projectcontrollertest.cpp )
add_executable(shell-projectcontrollertest ${projectcontrollertest_SRCS})
add_test(shell-projectcontrollertest shell-projectcontrollertest)
ecm_mark_as_test(shell-projectcontrollertest)
target_link_libraries(shell-projectcontrollertest Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KDev::Shell KDev::Sublime KDev::Project KDev::Interfaces )


set( test_sessioncontroller_SRCS test_sessioncontroller.cpp )
add_executable(test_sessioncontroller ${test_sessioncontroller_SRCS})
add_test(test_sessioncontroller test_sessioncontroller)
ecm_mark_as_test(test_sessioncontroller)
target_link_libraries(test_sessioncontroller Qt5::Test KF5::KDELibs4Support KF5::KIOWidgets KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime )

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
configure_file(share/kde4/services/kdevnonguiinterfaceplugin.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kservices5/kdevnonguiinterfaceplugin.desktop)
set( plugincontrollertest_plugin_SRCS nonguiinterfaceplugin.cpp )
add_library( kdevnonguiinterfaceplugin MODULE ${plugincontrollertest_plugin_SRCS} )
target_link_libraries( kdevnonguiinterfaceplugin ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBRARY}  KDev::Interfaces )

# Disabled for now, until I find a way of loading plugins from builddir/sourcedir
set( test_plugincontroller_SRCS test_plugincontroller.cpp )
add_executable(test_plugincontroller ${test_plugincontroller_SRCS})
add_test(test_plugincontroller test_plugincontroller)
ecm_mark_as_test(test_plugincontroller)
target_link_libraries(test_plugincontroller Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime )

set( test_testcontroller_SRCS test_testcontroller.cpp )
add_executable(test_testcontroller ${test_testcontroller_SRCS})
add_test(test_testcontroller test_testcontroller)
ecm_mark_as_test(test_testcontroller)
target_link_libraries(test_testcontroller Qt5::Test KDev::Tests)
