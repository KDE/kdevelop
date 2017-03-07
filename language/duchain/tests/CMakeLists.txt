ecm_add_test(test_duchain.cpp
    LINK_LIBRARIES KF5::TextEditor Qt5::Test KDev::Tests KDev::Language)

ecm_add_test(test_duchainshutdown.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)

ecm_add_test(test_identifier.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)

ecm_add_test(test_stringhelpers.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)

if(NOT COMPILER_OPTIMIZATIONS_DISABLED)
    ecm_add_test(bench_hashes.cpp
        LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)
    set_tests_properties(bench_hashes PROPERTIES TIMEOUT 30)
endif()
