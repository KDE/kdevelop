# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/shell
    )

ecm_add_test(test_documentcontroller.cpp
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(test_uicontroller.cpp
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(test_shellbuddy.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_shelldocumentoperation.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

## This test needs fixing:
# creating a factory is not enough to inject a new project plugin, we also need a KPluginInfo
# also need to adapt to naming scheme (test_projectcontroller.cpp)
ecm_add_test(test_projectcontroller.cpp
    TEST_NAME test_projectcontroller
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KDev::Shell KDev::Sublime KDev::Project KDev::Interfaces)

ecm_add_test(test_sessioncontroller.cpp
    LINK_LIBRARIES Qt5::Test KF5::KDELibs4Support KF5::KIOWidgets KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
configure_file(share/kde4/services/kdevnonguiinterfaceplugin.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kservices5/kdevnonguiinterfaceplugin.desktop)
set( plugincontrollertest_plugin_SRCS nonguiinterfaceplugin.cpp )
add_library( kdevnonguiinterfaceplugin MODULE ${plugincontrollertest_plugin_SRCS} )
target_link_libraries( kdevnonguiinterfaceplugin ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBRARY}  KDev::Interfaces )

ecm_add_test(test_plugincontroller.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_testcontroller.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests)
