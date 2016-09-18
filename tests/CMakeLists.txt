# ecm_add_test(test_plugin.cpp ${PROJECT_SOURCE_DIR}/src/plugin.cpp ${PROJECT_SOURCE_DIR}/src/job.cpp 
# ${PROJECT_SOURCE_DIR}/src/debug.cpp
#     TEST_NAME test_clangtidyplugin
#     LINK_LIBRARIES Qt5::Test KDev::Tests)

file(COPY "${PROJECT_SOURCE_DIR}/tests/output_example" 
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
     
ecm_add_test(test_clangtidyparser.cpp ../src/parsers/clangtidyparser.cpp ../src/debug.cpp
    TEST_NAME test_clangtidyparser
    LINK_LIBRARIES Qt5::Test KDev::Tests)

ecm_add_test(test_clangtidyjob.cpp ../src/job.cpp ../src/parsers/clangtidyparser.cpp ../src/debug.cpp
    TEST_NAME test_clangtidyjob
    LINK_LIBRARIES Qt5::Test KDev::Tests)
