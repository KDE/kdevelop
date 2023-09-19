remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_documentcontroller.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests)

ecm_add_test(test_uicontroller.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests)

ecm_add_test(test_shellbuddy.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_shelldocumentoperation.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_projectcontroller.cpp
    TEST_NAME test_projectcontroller
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Sublime KDev::Project KDev::Interfaces)

ecm_add_test(test_sessioncontroller.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF${KF_MAJOR_VERSION}::KIOWidgets KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

set(TEST_PLUGIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/testplugindir")
configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)

macro(KDEVSHELL_ADD_TEST_PLUGIN _PLUGIN_NAME)
    kdevplatform_add_plugin(${_PLUGIN_NAME} SKIP_INSTALL SOURCES plugins/${_PLUGIN_NAME}.cpp)
    target_link_libraries(${_PLUGIN_NAME} Qt${QT_MAJOR_VERSION}::Core KDev::Interfaces)
    set_target_properties(${_PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TEST_PLUGIN_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}")
endmacro()

kdevshell_add_test_plugin(nonguiinterfaceplugin)
kdevshell_add_test_plugin(projectdefaultplugin)
kdevshell_add_test_plugin(projectnondefaultplugin)
kdevshell_add_test_plugin(globaldefaultplugin)
kdevshell_add_test_plugin(globalnondefaultplugin)

ecm_add_test(test_plugincontroller.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_pluginenabling.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_testcontroller.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests)

ecm_add_test(test_workingsets.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell kdevworkingsets)

ecm_add_test(test_ktexteditorpluginintegration.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_detectedproblem.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell KDev::Serialization)

ecm_add_test(test_problemmodelset.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemstorenode.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemstore.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell)

ecm_add_test(test_filteredproblemstore.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemmodel.cpp
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDev::Shell)

add_library(languagecontrollertestbase STATIC
    languagecontrollertestbase.cpp)
target_link_libraries(languagecontrollertestbase PUBLIC
    Qt${QT_MAJOR_VERSION}::Test KDev::Tests)

ecm_add_test(test_languagecontroller.cpp LINK_LIBRARIES languagecontrollertestbase)
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(test_languagecontroller Threads::Threads)
endif()

if(BUILD_BENCHMARKS)
    ecm_add_test(bench_languagecontroller.cpp LINK_LIBRARIES languagecontrollertestbase)
    set_tests_properties(bench_languagecontroller PROPERTIES TIMEOUT 30)
endif()
