include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

add_executable(qml-parser
    qml-parser.cpp
)
target_link_libraries(qml-parser
    KDev::Tests
    kdevqmljsduchain
)

########### next target ###############

set(filestest_SRCS test_files.cpp)
add_executable(filestest ${filestest_SRCS})
add_test(filestest filestest)
configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
target_link_libraries(filestest
    Qt5::Test
    Qt5::Core
    KDev::Language
    KDev::Tests
    KDev::JsonTests
    kdevqmljsduchain
)
