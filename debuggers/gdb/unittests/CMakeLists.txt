ecm_add_test(test_gdb.cpp
    LINK_LIBRARIES kdevgdb_static KDev::Tests Qt5::Test
)
ecm_add_test(test_gdbprinters.cpp
    LINK_LIBRARIES
        Qt5::Core
        Qt5::Test
)

if (HAVE_PATH_WITH_SPACES_TEST)
    set_target_properties(test_gdb PROPERTIES COMPILE_FLAGS "-D")
endif()
