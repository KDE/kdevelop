add_debuggable_executable(debugee SRCS debugee.cpp)
add_debuggable_executable(debugeeslow SRCS debugeeslow.cpp)
add_debuggable_executable(debugeecrash SRCS debugeecrash.cpp)
add_debuggable_executable(debugeerecursion SRCS debugeerecursion.cpp)
add_debuggable_executable(debugeespace SRCS "debugee space.cpp")
add_debuggable_executable(debugeemultilocbreakpoint SRCS debugeemultilocbreakpoint.cpp)
add_debuggable_executable(debugeemultiplebreakpoint SRCS debugeemultiplebreakpoint.cpp)

add_debuggable_executable(debugeethreads SRCS debugeethreads.cpp)
target_link_libraries(debugeethreads ${QT_QTCORE_LIBRARY})

add_debuggable_executable(debugeeqt SRCS debugeeqt.cpp)
target_link_libraries(debugeeqt ${QT_QTCORE_LIBRARY})

add_debuggable_executable(debugeeexception SRCS debugeeexception.cpp)
set_target_properties(debugeeexception PROPERTIES COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory("path with space")
