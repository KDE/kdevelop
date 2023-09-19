remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(
    ../
    "${CMAKE_CURRENT_BINARY_DIR}/.."
)

ecm_add_test(test_replacementparser.cpp
    ../parsers/replacementparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_replacementparser
    LINK_LIBRARIES Qt::Test KDev::Tests KDevCompileAnalyzerCommon
)

ecm_add_test(test_clangtidyparser.cpp
    ../parsers/clangtidyparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_clangtidyparser
    LINK_LIBRARIES Qt::Test KDev::Tests KDevCompileAnalyzerCommon
)

ecm_add_test(test_clangtidyjob.cpp
    ../job.cpp
    ../parsers/clangtidyparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_clangtidyjob
    LINK_LIBRARIES Qt::Test KDev::Tests KDevCompileAnalyzerCommon
)

ecm_add_test(test_checkgroup.cpp
    ../config/checkgroup.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_checkgroup
    LINK_LIBRARIES Qt::Test KDev::Tests KDevCompileAnalyzerCommon
)

# TODO: Discover how to test the plugin class.
# ecm_add_test(test_plugin.cpp ${PROJECT_SOURCE_DIR}/src/plugin.cpp ${PROJECT_SOURCE_DIR}/src/job.cpp 
#     ${kdevclangtidy_LOG_SRCS}
#     TEST_NAME test_clangtidyplugin
#     LINK_LIBRARIES Qt::Test KDev::Tests)
