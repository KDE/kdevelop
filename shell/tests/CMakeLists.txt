# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/shell
    ${KDevPlatform_BINARY_DIR}/serialization
    )

ecm_add_test(test_documentcontroller.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests)

ecm_add_test(test_uicontroller.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests)

ecm_add_test(test_shellbuddy.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_shelldocumentoperation.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_projectcontroller.cpp
    TEST_NAME test_projectcontroller
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Sublime KDev::Project KDev::Interfaces)

ecm_add_test(test_sessioncontroller.cpp
    LINK_LIBRARIES Qt5::Test KF5::KIOWidgets KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
kdevplatform_add_plugin(kdevnonguiinterfaceplugin JSON kdevnonguiinterfaceplugin.json SKIP_INSTALL SOURCES nonguiinterfaceplugin.cpp)
target_link_libraries(kdevnonguiinterfaceplugin Qt5::Core KDev::Interfaces)

ecm_add_test(test_plugincontroller.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_testcontroller.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests)

ecm_add_test(test_ktexteditorpluginintegration.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_detectedproblem.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell KDev::Serialization)

ecm_add_test(test_problemmodelset.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemstorenode.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemstore.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell)

ecm_add_test(test_filteredproblemstore.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemmodel.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell)

ecm_add_test(test_checkerstatus.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell)
