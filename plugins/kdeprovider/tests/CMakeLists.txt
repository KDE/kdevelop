remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

set(test_kdeprojectsreader_SRCS
    ../kdeprojectsreader.cpp
    ../kdeprojectsmodel.cpp
    test_kdeprojectsreader.cpp)
ecm_add_test(${test_kdeprojectsreader_SRCS}
    TEST_NAME test_kdeprojectsreader
    LINK_LIBRARIES Qt5::Test Qt5::Core Qt5::Gui Qt5::Network KDev::Vcs)
