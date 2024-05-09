remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_docker.cpp
        LINK_LIBRARIES Qt::Test Qt::Core KDev::Interfaces KDevPlatformTests)
