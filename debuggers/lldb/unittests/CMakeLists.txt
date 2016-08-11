# Unit tests
add_subdirectory(debugees)

set(test_lldb_SRCS
    test_lldb.cpp
    testhelper.cpp
)

ecm_add_test(${test_lldb_SRCS}
    TEST_NAME test_lldb
    LINK_LIBRARIES
        kdevlldb_static
        Qt5::Core
        Qt5::Test
        KDev::Shell
        KDev::Interfaces
        KDev::Project
        KDev::Debugger
        KDev::Tests
        KDev::Util
        KF5::KIOWidgets
        KF5::TextEditor
        KF5::Parts
)

if (HAVE_PATH_WITH_SPACES_TEST)
    set_target_properties(test_lldb PROPERTIES COMPILE_FLAGS "-DHAVE_PATH_WITH_SPACES_TEST")
endif()

# Unit tests for printers
ecm_add_test(test_lldbformatters.cpp testhelper.cpp
    TEST_NAME test_lldbformatters
    LINK_LIBRARIES
        kdevlldb_static
        Qt5::Core
        Qt5::Test
        KDev::Shell
        KDev::Interfaces
        KDev::Project
        KDev::Debugger
        KDev::Tests
        KDev::Util
        KF5::KIOWidgets
        KF5::TextEditor
        KF5::Parts
)
