add_debuggable_executable(lldb_debugee SRCS debugee.cpp)
add_debuggable_executable(lldb_debugeeslow SRCS debugeeslow.cpp)
add_debuggable_executable(lldb_debugeecrash SRCS debugeecrash.cpp)
add_debuggable_executable(lldb_debugeerecursion SRCS debugeerecursion.cpp)
add_debuggable_executable(lldb_debugeespace SRCS "debugee space.cpp")
add_debuggable_executable(lldb_debugeemultilocbreakpoint SRCS debugeemultilocbreakpoint.cpp)
add_debuggable_executable(lldb_debugeemultiplebreakpoint SRCS debugeemultiplebreakpoint.cpp)
add_debuggable_executable(lldb_debugeeechoenv SRCS debugeeechoenv.cpp)

add_debuggable_executable(lldb_debugeethreads SRCS debugeethreads.cpp)
target_link_libraries(lldb_debugeethreads Qt5::Core)

add_debuggable_executable(lldb_debugeeqt SRCS debugeeqt.cpp)
target_link_libraries(lldb_debugeeqt Qt5::Core)

add_debuggable_executable(lldb_debugeeexception SRCS debugeeexception.cpp)
kde_target_enable_exceptions(lldb_debugeeexception PRIVATE)

if (HAVE_PATH_WITH_SPACES_TEST)
    add_subdirectory("path with space")
endif()
