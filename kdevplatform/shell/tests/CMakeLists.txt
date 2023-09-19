remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_documentcontroller.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests)

ecm_add_test(test_uicontroller.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests)

ecm_add_test(test_shellbuddy.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_shelldocumentoperation.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_projectcontroller.cpp
    TEST_NAME test_projectcontroller
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Sublime KDev::Project KDev::Interfaces)

ecm_add_test(test_sessioncontroller.cpp
    LINK_LIBRARIES Qt::Test KF6::KIOWidgets KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

set(TEST_PLUGIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/testplugindir")
configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)

macro(KDEVSHELL_ADD_TEST_PLUGIN _PLUGIN_NAME)
    set(_plugin "test_${_PLUGIN_NAME}")
    kdevplatform_add_plugin(${_plugin} SKIP_INSTALL SOURCES plugins/${_PLUGIN_NAME}plugin.cpp)
    target_link_libraries(${_plugin} Qt::Core KDev::Interfaces)
    set_target_properties(${_plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TEST_PLUGIN_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}")
endmacro()

kdevshell_add_test_plugin(nonguiinterface)
kdevshell_add_test_plugin(projectdefault)
kdevshell_add_test_plugin(projectnondefault)
kdevshell_add_test_plugin(globaldefault)
kdevshell_add_test_plugin(globalnondefault)

ecm_add_test(test_plugincontroller.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_pluginenabling.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_testcontroller.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests)

ecm_add_test(test_workingsets.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell kdevworkingsets)

ecm_add_test(test_ktexteditorpluginintegration.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Interfaces KDev::Sublime)

ecm_add_test(test_detectedproblem.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Serialization)

ecm_add_test(test_problemmodelset.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemstorenode.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemstore.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell)

ecm_add_test(test_filteredproblemstore.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell)

ecm_add_test(test_problemmodel.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell)

add_library(languagecontrollertestbase STATIC
    languagecontrollertestbase.cpp)
target_link_libraries(languagecontrollertestbase PUBLIC
    Qt::Test KDev::Tests)

ecm_add_test(test_languagecontroller.cpp LINK_LIBRARIES languagecontrollertestbase)
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(test_languagecontroller Threads::Threads)
endif()

if(BUILD_BENCHMARKS)
    ecm_add_test(bench_languagecontroller.cpp LINK_LIBRARIES languagecontrollertestbase)
    set_tests_properties(bench_languagecontroller PROPERTIES TIMEOUT 30)
endif()
