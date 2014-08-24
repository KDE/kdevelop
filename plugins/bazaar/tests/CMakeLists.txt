# Running the test only makes sense if the bzr command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(BZR_FOUND NAMES bzr)

if (BZR_FOUND)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    set(bazaartest_SRCS
        bazaartest.cpp
        ../diffjob.cpp
        ../bzrannotatejob.cpp
        ../bazaarplugin.cpp
        ../bazaarutils.cpp
        ../copyjob.cpp
    )

    ecm_add_test(${bazaartest_SRCS}
        TEST_NAME kdevbazaar-test
        LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_VCS_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})
endif()
