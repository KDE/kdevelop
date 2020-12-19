remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

get_filename_component(GDB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../gdb ABSOLUTE)
get_filename_component(LLDB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lldb ABSOLUTE)
configure_file(debuggers-tests-config.h.in debuggers-tests-config.h)

add_subdirectory(debuggees)

add_library(kdevdbg_testhelper STATIC testhelper.cpp)
target_link_libraries(kdevdbg_testhelper
    PUBLIC
    kdevdebuggercommon
    KDev::Debugger
    KDev::IExecute
    KDev::Interfaces
    Qt5::Core
    PRIVATE
    KDev::Util
    Qt5::Test
)

ecm_add_test(test_miparser
    LINK_LIBRARIES Qt5::Test kdevdebuggercommon
)

ecm_add_test(test_micommand
    LINK_LIBRARIES Qt5::Test kdevdebuggercommon
)

ecm_add_test(test_micommandqueue
    LINK_LIBRARIES Qt5::Test kdevdebuggercommon
)

set(TEST_PLUGIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/testplugindir")
configure_file("plugins/testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)

kdevplatform_add_plugin(testdebugger JSON plugins/testdebugger.json SKIP_INSTALL SOURCES plugins/testdebugger.cpp)
target_link_libraries(testdebugger kdevdebuggercommon)
set_target_properties(testdebugger PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TEST_PLUGIN_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}")

ecm_add_test(test_midbus
    LINK_LIBRARIES Qt5::Test Qt5::DBus Qt5::Core kdevdebuggercommon KDevPlatformTests
)
