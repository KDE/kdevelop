remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_vcsrevision
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcsannotationline
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcsannotation
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcsitemevent
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcsevent
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcsstatusinfo
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcsdiff
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)

ecm_add_test(test_vcslocation
    LINK_LIBRARIES Qt5::Test KDev::Vcs
)
