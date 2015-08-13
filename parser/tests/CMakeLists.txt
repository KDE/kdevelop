set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

ecm_add_test(parsetest.cpp ../../debug.cpp
    TEST_NAME test_qmakeparse
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)

ecm_add_test(assignmenttest.cpp ../../debug.cpp
    TEST_NAME test_assignment
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)

ecm_add_test(functionscopetest.cpp testhelpers.cpp ../../debug.cpp
    TEST_NAME test_functionscope
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
    TEST_NAME test_lexer
    LINK_LIBRARIES
        Qt5::Test
        kdevqmakeparser
)
