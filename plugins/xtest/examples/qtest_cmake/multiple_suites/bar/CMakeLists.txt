include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(mootest_SRCS bartest.cpp)
qt4_automoc(${mootest_SRCS})
add_test(mootest mootest)
add_executable(mootest ${mootest_SRCS})
target_link_libraries(mootest ${QT_QTTEST_LIBRARY})
