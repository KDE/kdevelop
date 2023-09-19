remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

add_library(quickopentestbase STATIC
    quickopentestbase.cpp
    ../projectfilequickopen.cpp)

target_link_libraries(quickopentestbase PUBLIC
    KDev::Tests
    KDev::Project
    KDev::Language
    gfx::timsort
    Qt::Test
)

ecm_add_test(test_quickopen.cpp LINK_LIBRARIES quickopentestbase)

if(BUILD_BENCHMARKS)
    ecm_add_test(bench_quickopen.cpp LINK_LIBRARIES quickopentestbase)
    set_tests_properties(bench_quickopen PROPERTIES TIMEOUT 30)
endif()
