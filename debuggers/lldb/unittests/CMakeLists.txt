set(test_lldb_SRCS
    test_lldb.cpp
    testhelper.cpp
)

ecm_add_test(${test_lldb_SRCS}
    TEST_NAME test_lldb
    LINK_LIBRARIES
        kdevlldb_static
        KDev::Tests
        Qt5::Test
)

# Unit tests for printers
ecm_add_test(test_lldbformatters.cpp testhelper.cpp
    TEST_NAME test_lldbformatters
    LINK_LIBRARIES
        kdevlldb_static
        KDev::Tests
        Qt5::Test
)
