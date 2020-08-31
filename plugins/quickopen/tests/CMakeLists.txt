remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

add_library(quickopentestbase STATIC
    quickopentestbase.cpp
    ../projectfilequickopen.cpp
    ${kdevquickopen_LOG_SRCS}
)

target_link_libraries(quickopentestbase PUBLIC
    KDev::Tests
    KDev::Project
    KDev::Language
    Qt5::Test
)

ecm_add_test(test_quickopen.cpp LINK_LIBRARIES quickopentestbase)

if(NOT COMPILER_OPTIMIZATIONS_DISABLED)
    ecm_add_test(bench_quickopen.cpp LINK_LIBRARIES quickopentestbase)
    set_tests_properties(bench_quickopen PROPERTIES TIMEOUT 30)
endif()
