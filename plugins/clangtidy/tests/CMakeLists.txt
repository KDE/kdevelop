remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ../
    "${CMAKE_CURRENT_BINARY_DIR}/.."
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

# disable for now:
# CentOS used for appimage does not have string_ref.hpp (Boost >= 1.53) as used by current code
if(FALSE)
ecm_add_test(test_replacementparser.cpp
    ../parsers/replacementparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_replacementparser
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDevCompileAnalyzerCommon
)
endif()

ecm_add_test(test_clangtidyparser.cpp
    ../parsers/clangtidyparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_clangtidyparser
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDevCompileAnalyzerCommon
)

ecm_add_test(test_clangtidyjob.cpp
    ../job.cpp
    ../parsers/clangtidyparser.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_clangtidyjob
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDevCompileAnalyzerCommon
)

ecm_add_test(test_checkgroup.cpp
    ../config/checkgroup.cpp
    ${kdevclangtidy_LOG_SRCS}
    TEST_NAME test_checkgroup
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests KDevCompileAnalyzerCommon
)

# TODO: Discover how to test the plugin class.
# ecm_add_test(test_plugin.cpp ${PROJECT_SOURCE_DIR}/src/plugin.cpp ${PROJECT_SOURCE_DIR}/src/job.cpp 
#     ${kdevclangtidy_LOG_SRCS}
#     TEST_NAME test_clangtidyplugin
#     LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KDev::Tests)
