remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

ecm_add_test(test_duchain.cpp
    LINK_LIBRARIES KF6::TextEditor Qt::Test KDev::Tests KDev::Language)

ecm_add_test(test_duchainshutdown.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)

ecm_add_test(test_identifier.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)
add_target_compile_flag_if_supported(test_identifier PRIVATE -Wno-self-assign-overloaded) # self-assignments are on purpose here

ecm_add_test(test_stringhelpers.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)

if(BUILD_BENCHMARKS)
    ecm_add_test(bench_hashes.cpp
        LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)
    set_tests_properties(bench_hashes PROPERTIES TIMEOUT 30)
endif()
