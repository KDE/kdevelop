include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(footest_SRCS footest.cpp)
qt4_automoc(${footest_SRCS})
add_test(footest footest)
add_executable(footest ${footest_SRCS})
target_link_libraries(footest ${QT_QTTEST_LIBRARY})

add_subdirectory(bar)
