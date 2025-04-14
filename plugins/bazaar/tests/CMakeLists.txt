remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

set(bazaartest_SRCS
    test_bazaar.cpp
    ../diffjob.cpp
    ../bzrannotatejob.cpp
    ../bazaarplugin.cpp
    ../bazaarutils.cpp
    ../copyjob.cpp
)

ecm_add_test(${bazaartest_SRCS}
    TEST_NAME test_kdevbazaar
    LINK_LIBRARIES Qt::Test KDev::Vcs KDev::Util KDev::Tests)
