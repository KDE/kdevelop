remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_gdb.cpp
    LINK_LIBRARIES
        kdevgdb_static
        kdevdbg_testhelper
        KDev::Tests
        Qt::Test
)

ecm_add_test(test_gdbprinters.cpp
    LINK_LIBRARIES
        Qt::Core
        Qt::Test
        kdevdebuggercommon
)
