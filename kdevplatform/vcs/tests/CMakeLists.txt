remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_vcsrevision.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcsannotationline.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcsannotation.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcsitemevent.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcsevent.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcsstatusinfo.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcsdiff.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)

ecm_add_test(test_vcslocation.cpp
    LINK_LIBRARIES Qt::Test KDev::Vcs
)
