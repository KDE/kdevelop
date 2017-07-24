ecm_add_test(test_lldb.cpp
    LINK_LIBRARIES
        kdevlldb_static
        kdevdbg_testhelper
        KDev::Tests
        Qt5::Test
)

# Unit tests for printers
ecm_add_test(test_lldbformatters.cpp
    LINK_LIBRARIES
        kdevlldb_static
        kdevdbg_testhelper
        KDev::Tests
        Qt5::Test
)
