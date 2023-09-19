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
    Qt::Core
    PRIVATE
    KDev::Tests
    KDev::Util
    Qt::Test
)

ecm_add_test(test_miparser.cpp
    LINK_LIBRARIES Qt::Test kdevdebuggercommon
)

ecm_add_test(test_micommand.cpp
    LINK_LIBRARIES Qt::Test kdevdebuggercommon
)

ecm_add_test(test_micommandqueue.cpp
    LINK_LIBRARIES Qt::Test kdevdebuggercommon
)

set(TEST_PLUGIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/testplugindir")
configure_file("plugins/testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)

kdevplatform_add_plugin(testdebugger SKIP_INSTALL SOURCES plugins/testdebugger.cpp)
target_link_libraries(testdebugger kdevdebuggercommon KDev::Tests)
set_target_properties(testdebugger PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TEST_PLUGIN_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}")

ecm_add_test(test_midbus.cpp
    LINK_LIBRARIES Qt::Test Qt::DBus Qt::Core kdevdebuggercommon KDevPlatformTests
)
