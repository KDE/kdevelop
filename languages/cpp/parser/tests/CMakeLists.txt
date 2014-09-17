configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

ecm_add_test(test_parser.cpp test_parser_cpp2011.cpp TEST_NAME test_parser
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test KDev::Language KDev::Tests kdevcpprpp kdevcppparser)

ecm_add_test(test_generator.cpp TEST_NAME test_generator
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test KDev::Tests kdevcpprpp kdevcppparser)

ecm_add_test(test_pool.cpp TEST_NAME test_pool
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test kdevcppparser)


