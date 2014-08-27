include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_BINARY_DIR}/languages/cpp
)

ecm_add_test(test_expressionparser.cpp testhelper.cpp TEST_NAME cppexpressionparsertest
LINK_LIBRARIES
    kdevcppparser kdevcpprpp Qt5::Test ${KDEVPLATFORM_LANGUAGE_LIBRARIES} kdevcppduchain ${KDEVPLATFORM_TESTS_LIBRARIES}  )


include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_SOURCE_DIR}/languages/cpp
)

ecm_add_test(test_duchain.cpp test_duchain_cpp2011.cpp testhelper.cpp TEST_NAME duchaintest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(test_sourcemanipulation.cpp testhelper.cpp TEST_NAME sourcemanipulationtest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(codeanalysistest.cpp testhelper.cpp TEST_NAME codeanalysistest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_add_test(testenvironment.cpp testhelper.cpp TEST_NAME environmenttest
LINK_LIBRARIES
    KF5::TextEditor Qt5::Test
    kdevcppparser kdevcpprpp kdevcppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})
