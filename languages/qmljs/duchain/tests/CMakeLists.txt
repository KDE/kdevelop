include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(testcontexts_SRCS testcontexts.cpp)

add_executable(testcontexts ${testcontexts_SRCS})
add_test(testcontexts testcontexts)
target_link_libraries(testcontexts
    Qt5::Test
    KDev::Language
    KDev::Tests
    kdevqmljsduchain
)

########### next target ###############

set(testdeclarations_SRCS testdeclarations.cpp)

add_executable(testdeclarations ${testdeclarations_SRCS})
add_test(testdeclarations testdeclarations)
target_link_libraries(testdeclarations
    Qt5::Test
    KDev::Language
    KDev::Tests
    kdevqmljsduchain
)
