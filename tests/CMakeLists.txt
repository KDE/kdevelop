remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ../src
)

file(COPY "${PROJECT_SOURCE_DIR}/tests/data" 
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")


ecm_add_test(test_replacementparser.cpp
    ../src/parsers/replacementparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_replacementparser
    LINK_LIBRARIES Qt5::Test KDev::Tests
)

ecm_add_test(test_clangtidyparser.cpp
    ../src/parsers/clangtidyparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_clangtidyparser
    LINK_LIBRARIES Qt5::Test KDev::Tests
)

ecm_add_test(test_clangtidyjob.cpp
    ../src/job.cpp
    ../src/parsers/clangtidyparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_clangtidyjob
    LINK_LIBRARIES Qt5::Test KDev::Tests
)

# TODO: Discover how to test the plugin class.
# ecm_add_test(test_plugin.cpp ${PROJECT_SOURCE_DIR}/src/plugin.cpp ${PROJECT_SOURCE_DIR}/src/job.cpp 
#     ${kdevclangtidy_LOG_SRCS}
#     TEST_NAME test_clangtidyplugin
#     LINK_LIBRARIES Qt5::Test KDev::Tests)
