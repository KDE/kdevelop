# The executables go into the tests dir
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/shell
    )

set(documentcontrollertest_SRCS documentcontrollertest.cpp)
kde4_add_unit_test(shell-documentcontrollertest ${documentcontrollertest_SRCS})
target_link_libraries( shell-documentcontrollertest ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES})

set(uicontrollertest_SRCS uicontrollertest.cpp)
kde4_add_unit_test(shell-uicontrollertest ${uicontrollertest_SRCS})
target_link_libraries( shell-uicontrollertest ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES})

set( shellbuddytest_SRCS shellbuddytest.cpp )
kde4_add_unit_test(shell-buddytest ${shellbuddytest_SRCS})
target_link_libraries(shell-buddytest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

set( shelldocumentoperationtest_SRCS shelldocumentoperationtest.cpp )
kde4_add_unit_test(shell-documentoperationtest ${shelldocumentoperationtest_SRCS})
target_link_libraries(shell-documentoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

## This test needs fixing:
# creating a factory is not enough to inject a new project plugin, we also need a KPluginInfo
set( projectcontrollertest_SRCS projectcontrollertest.cpp )
#kde4_add_unit_test(shell-projectcontrollertest ${projectcontrollertest_SRCS})
#target_link_libraries(shell-projectcontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime kdevplatformproject kdevplatforminterfaces )

set( sessioncontrollertest_SRCS sessioncontrollertest.cpp )
kde4_add_unit_test(shell-sessioncontrollertest ${sessioncontrollertest_SRCS})
target_link_libraries(shell-sessioncontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
configure_file(share/kde4/services/kdevnonguiinterfaceplugin.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/share/kde4/services/kdevnonguiinterfaceplugin.desktop)
set( plugincontrollertest_plugin_SRCS nonguiinterfaceplugin.cpp )
kde4_add_plugin( kdevnonguiinterfaceplugin ${plugincontrollertest_plugin_SRCS} )
target_link_libraries( kdevnonguiinterfaceplugin ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBRARY}  kdevplatforminterfaces )

# Disabled for now, until I find a way of loading plugins from builddir/sourcedir
set( plugincontrollertest_SRCS plugincontrollertest.cpp )
kde4_add_unit_test(shell-plugincontrollertest ${plugincontrollertest_SRCS})
target_link_libraries(shell-plugincontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformtests kdevplatformshell kdevplatforminterfaces sublime )

set( testcontrollertest_SRCS testcontrollertest.cpp )
kde4_add_unit_test(shell-testcontrollertest ${testcontrollertest_SRCS})
target_link_libraries(shell-testcontrollertest ${QT_QTTEST_LIBRARY} kdevplatformtests)
