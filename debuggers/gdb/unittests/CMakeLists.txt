ecm_add_test(test_gdb.cpp
    LINK_LIBRARIES
        kdevgdb_static
        kdevdbg_testhelper
        KDev::Tests
        Qt5::Test
)

ecm_add_test(test_gdbprinters.cpp
    LINK_LIBRARIES
        Qt5::Core
        Qt5::Test
)
