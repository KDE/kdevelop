ecm_add_test(parsetest.cpp ../../debug.cpp
    TEST_NAME test_qmakeparse
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)

ecm_add_test(assignmenttest.cpp ../../debug.cpp
    TEST_NAME test_qmakeassignment
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)

ecm_add_test(functionscopetest.cpp testhelpers.cpp ../../debug.cpp
    TEST_NAME test_qmakefunctionscope
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)

ecm_add_test(scopetest.cpp testhelpers.cpp ../../debug.cpp
    TEST_NAME test_qmakescope
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)

ecm_add_test(lexertest.cpp
    TEST_NAME test_qmakelexer
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)
