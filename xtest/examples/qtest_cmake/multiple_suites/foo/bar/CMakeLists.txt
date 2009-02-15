include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(bartest_SRCS bartest.cpp)
qt4_automoc(${bartest_SRCS})
add_test(bartest bartest)
add_executable(bartest ${bartest_SRCS})
target_link_libraries(bartest ${QT_QTTEST_LIBRARY})

set(baztest_SRCS baztest.cpp)
qt4_automoc(${baztest_SRCS})
add_test(baztest baztest)
add_executable(baztest ${baztest_SRCS})
target_link_libraries(baztest ${QT_QTTEST_LIBRARY})
