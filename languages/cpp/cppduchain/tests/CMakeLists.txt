include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_BINARY_DIR}/languages/cpp
)

ecm_add_test(test_expressionparser.cpp test_helper.cpp TEST_NAME test_cppexpressionparser
LINK_LIBRARIES
    kdevcppparser kdevcpprpp Qt5::Test ${KDEVPLATFORM_LANGUAGE_LIBRARIES} kdevcppduchain ${KDEVPLATFORM_TESTS_LIBRARIES}  )


include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_SOURCE_DIR}/languages/cpp
)

ecm_add_test(test_duchain.cpp test_duchain_cpp2011.cpp test_helper.cpp TEST_NAME test_duchain
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(test_sourcemanipulation.cpp test_helper.cpp TEST_NAME test_sourcemanipulation
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(test_codeanalysis.cpp test_helper.cpp TEST_NAME test_codeanalysis
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(test_environment.cpp test_helper.cpp TEST_NAME test_environment
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})
