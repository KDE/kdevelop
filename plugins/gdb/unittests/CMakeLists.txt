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
        Qt${QT_MAJOR_VERSION}::Test
)

ecm_add_test(test_gdbprinters.cpp
    LINK_LIBRARIES
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Test
        kdevdebuggercommon
)
