ecm_add_test(test_embeddedfreetree.cpp
    LINK_LIBRARIES KF5::TextEditor Qt5::Test KDev::Language KDev::Tests)

ecm_add_test(test_kdevvarlengtharray.cpp
    LINK_LIBRARIES Qt5::Test)

ecm_add_test(test_objectlist.cpp
    LINK_LIBRARIES Qt5::Test KDev::Util)

ecm_add_test(test_stringhandler.cpp
    LINK_LIBRARIES Qt5::Test KDev::Util)

ecm_add_test(test_path.cpp
    LINK_LIBRARIES Qt5::Test KF5::KIOCore KDev::Tests KDev::Util)

ecm_add_test(test_foregroundlock.cpp
    LINK_LIBRARIES Qt5::Test KDev::Util)

ecm_add_test(test_executecompositejob.cpp
    LINK_LIBRARIES Qt5::Test KDev::Util)
