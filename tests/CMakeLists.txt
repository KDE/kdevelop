ecm_add_test(
    ../debug.cpp
    ../parser.cpp
    test_cppcheckparser.cpp

    TEST_NAME test_cppcheckparser
    LINK_LIBRARIES Qt5::Test KDev::Tests
)

kconfig_add_kcfg_files(test_cppcheckjob_CONFIG_SRCS
    ../config/globalsettings.kcfgc
    ../config/projectsettings.kcfgc
)

ecm_add_test(
    ${test_cppcheckjob_CONFIG_SRCS}
    ../debug.cpp
    ../job.cpp
    ../parameters.cpp
    ../parser.cpp
    test_cppcheckjob.cpp

    TEST_NAME test_cppcheckjob
    LINK_LIBRARIES Qt5::Test KDev::Tests
)
