get_filename_component(GDB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../gdb ABSOLUTE)
get_filename_component(LLDB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lldb ABSOLUTE)
configure_file(debuggers-tests-config.h.in debuggers-tests-config.h)

add_subdirectory(debuggees)

add_library(kdevdbg_testhelper STATIC testhelper.cpp)
target_link_libraries(kdevdbg_testhelper
    PUBLIC
    kdevdebuggercommon
    KDev::Debugger
    Qt5::Core
    PRIVATE
    Qt5::Test
)
