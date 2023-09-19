remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_embeddedfreetree.cpp
    LINK_LIBRARIES KF6::TextEditor Qt::Test KDev::Language KDev::Tests)

ecm_add_test(test_kdevvarlengtharray.cpp
    LINK_LIBRARIES Qt::Test)

ecm_add_test(test_objectlist.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(test_stringhandler.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(test_texteditorhelpers.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(test_path.cpp
    LINK_LIBRARIES Qt::Test KF6::KIOCore KDev::Tests KDev::Util)

ecm_add_test(test_foregroundlock.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(../kdevcoreaddons/kcompoundjobtest.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(../kdevcoreaddons/ksequentialcompoundjobtest.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(test_executecompositejob.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(test_environment.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)

ecm_add_test(
    ../kdevformatfile.cpp
    test_kdevformatsource.cpp 

    TEST_NAME test_kdevformatsource
    LINK_LIBRARIES Qt::Test Qt::Core5Compat KDev::Util
)

ecm_add_test(test_formattinghelpers.cpp
    LINK_LIBRARIES Qt::Test KDev::Util)
