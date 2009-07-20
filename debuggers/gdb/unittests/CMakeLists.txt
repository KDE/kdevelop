ADD_EXECUTABLE(debugee debugee.cpp)
ADD_EXECUTABLE(debugeeslow debugeeslow.cpp)
ADD_EXECUTABLE(debugeecrash debugeecrash.cpp)
ADD_EXECUTABLE(debugeerecursion debugeerecursion.cpp)

ADD_EXECUTABLE(debugeethreads debugeethreads.cpp)
target_link_libraries(debugeethreads ${QT_QTCORE_LIBRARY})
