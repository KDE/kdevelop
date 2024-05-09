remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(parsetest.cpp ${qmakecommon_LOG_SRCS}
    TEST_NAME test_qmakeparse
    LINK_LIBRARIES
        Qt::Test
        kdevqmakeparser
)

ecm_add_test(assignmenttest.cpp ${qmakecommon_LOG_SRCS}
    TEST_NAME test_qmakeassignment
    LINK_LIBRARIES
        Qt::Test
        kdevqmakeparser
)

ecm_add_test(functionscopetest.cpp testhelpers.cpp ${qmakecommon_LOG_SRCS}
    TEST_NAME test_qmakefunctionscope
    LINK_LIBRARIES
        Qt::Test
        kdevqmakeparser
)

ecm_add_test(scopetest.cpp testhelpers.cpp ${qmakecommon_LOG_SRCS}
    TEST_NAME test_qmakescope
    LINK_LIBRARIES
        Qt::Test
        kdevqmakeparser
)

ecm_add_test(lexertest.cpp
    TEST_NAME test_qmakelexer
    LINK_LIBRARIES
        Qt::Test
        kdevqmakeparser
)
