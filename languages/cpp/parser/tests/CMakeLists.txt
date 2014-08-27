configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

ecm_add_test(test_parser.cpp test_parser_cpp2011.cpp TEST_NAME parsertest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} kdevcpprpp kdevcppparser)

ecm_add_test(test_generator.cpp TEST_NAME generatortest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} kdevcpprpp kdevcppparser)

ecm_add_test(test_pool.cpp TEST_NAME pooltest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test kdevcppparser)


