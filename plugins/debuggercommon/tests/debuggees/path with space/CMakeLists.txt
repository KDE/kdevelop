# workaround https://gitlab.kitware.com/cmake/cmake/-/issues/25554
set(CMAKE_AUTOMOC OFF)

add_debuggable_executable(debuggee_spacedebugee SRCS "spacedebugee.cpp")
target_link_libraries(debuggee_spacedebugee Qt${QT_MAJOR_VERSION}::Core)
