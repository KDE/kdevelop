# The executables go into the tests dir
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/shell
    )

# Disabled until I find out how to load plugins from builddir
#set( plugincontrollertest_plugin_SRCS nonguiinterfaceplugin.cpp )
#add_library( kdevnonguiinterfaceplugin MODULE ${plugincontrollertest_plugin_SRCS} )
#target_link_libraries( kdevnonguiinterfaceplugin ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBRARY} kdevplatforminterfaces )

set(documentcontrollertest_SRCS documentcontrollertest.cpp)
add_executable(shell-documentcontrollertest ${documentcontrollertest_SRCS})
ecm_mark_as_test(shell-documentcontrollertest)
target_link_libraries( shell-documentcontrollertest ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES})

set(uicontrollertest_SRCS uicontrollertest.cpp)
add_executable(shell-uicontrollertest ${uicontrollertest_SRCS})
ecm_mark_as_test(shell-uicontrollertest)
target_link_libraries( shell-uicontrollertest ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES})

set( shellbuddytest_SRCS shellbuddytest.cpp )
add_executable(shell-buddytest ${shellbuddytest_SRCS})
ecm_mark_as_test(shell-buddytest)
target_link_libraries(shell-buddytest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

set( shelldocumentoperationtest_SRCS shelldocumentoperationtest.cpp )
add_executable(shell-documentoperationtest ${shelldocumentoperationtest_SRCS})
ecm_mark_as_test(shell-documentoperationtest)
target_link_libraries(shell-documentoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

## This test needs fixing:
# creating a factory is not enough to inject a new project plugin, we also need a KPluginInfo
set( projectcontrollertest_SRCS projectcontrollertest.cpp )
#add_executable(shell-projectcontrollertest ${projectcontrollertest_SRCS})\necm_mark_as_test(shell-projectcontrollertest)
#target_link_libraries(shell-projectcontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime kdevplatformproject kdevplatforminterfaces )

set( sessioncontrollertest_SRCS sessioncontrollertest.cpp )
add_executable(shell-sessioncontrollertest ${sessioncontrollertest_SRCS})
ecm_mark_as_test(shell-sessioncontrollertest)
target_link_libraries(shell-sessioncontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

# Disabled for now, until I find a way of loading plugins from builddir/sourcedir
#set( plugincontrollertest_SRCS plugincontrollertest.cpp )
#add_executable(shell-plugincontrollertest ${plugincontrollertest_SRCS})\necm_mark_as_test(shell-plugincontrollertest)
#target_link_libraries(shell-plugincontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatforminterfaces sublime )

set( testcontrollertest_SRCS testcontrollertest.cpp )
add_executable(shell-testcontrollertest ${testcontrollertest_SRCS})
ecm_mark_as_test(shell-testcontrollertest)
target_link_libraries(shell-testcontrollertest ${QT_QTTEST_LIBRARY} kdevplatformtests)
