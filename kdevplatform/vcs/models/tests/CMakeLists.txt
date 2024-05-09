remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_models.cpp
    LINK_LIBRARIES Qt::Test Qt::Gui KDev::Tests KDev::Util KDev::Vcs
    GUI)
