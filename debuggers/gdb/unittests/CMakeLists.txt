add_debuggable_executable(debugee SRCS debugee.cpp)
add_debuggable_executable(debugeeslow SRCS debugeeslow.cpp)
add_debuggable_executable(debugeecrash SRCS debugeecrash.cpp)
add_debuggable_executable(debugeerecursion SRCS debugeerecursion.cpp)
add_debuggable_executable(debugeespace SRCS "debugee space.cpp")
add_debuggable_executable(debugeemultilocbreakpoint SRCS debugeemultilocbreakpoint.cpp)
add_debuggable_executable(debugeemultiplebreakpoint SRCS debugeemultiplebreakpoint.cpp)

add_debuggable_executable(debugeethreads SRCS debugeethreads.cpp)
target_link_libraries(debugeethreads Qt5::Core)

add_debuggable_executable(debugeeqt SRCS debugeeqt.cpp)
target_link_libraries(debugeeqt Qt5::Core)

add_debuggable_executable(debugeeexception SRCS debugeeexception.cpp)
kde_target_enable_exceptions(debugeeexception PRIVATE)

add_subdirectory("path with space")
