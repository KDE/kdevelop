# force debug symbols for our debugees
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_BUILD_TYPE debugfull)
else()
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(debugee debugee.cpp)
add_executable(debugeeslow debugeeslow.cpp)
add_executable(debugeecrash debugeecrash.cpp)
add_executable(debugeerecursion debugeerecursion.cpp)

add_executable(debugeethreads debugeethreads.cpp)
target_link_libraries(debugeethreads ${QT_QTCORE_LIBRARY})

add_executable(debugeeqt debugeeqt.cpp)
target_link_libraries(debugeeqt ${QT_QTCORE_LIBRARY})

add_executable("debugeespace" "debugee space.cpp")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")
add_executable("debugeeexception" "debugeeexception.cpp")

add_executable(debugeemultilocbreakpoint debugeemultilocbreakpoint.cpp)

add_executable(debugeemultiplebreakpoint debugeemultiplebreakpoint.cpp)