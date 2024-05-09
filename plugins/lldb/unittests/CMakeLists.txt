remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_lldb.cpp
    LINK_LIBRARIES
        kdevlldb_static
        kdevdbg_testhelper
        KDev::Tests
        Qt::Test
)

# Unit tests for printers
ecm_add_test(test_lldbformatters.cpp
    LINK_LIBRARIES
        kdevlldb_static
        kdevdbg_testhelper
        KDev::Tests
        Qt::Test
)
