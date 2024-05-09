remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

# Running the test only makes sense if the bzr command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(BZR_FOUND NAMES bzr)

if (BZR_FOUND)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
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
endif()
