add_executable(debugee debugee.cpp)
add_executable(debugeeslow debugeeslow.cpp)
add_executable(debugeecrash debugeecrash.cpp)
add_executable(debugeerecursion debugeerecursion.cpp)

add_executable(debugeethreads debugeethreads.cpp)
target_link_libraries(debugeethreads ${QT_QTCORE_LIBRARY})
