include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

ecm_add_test(test_qmljscontexts.cpp
    LINK_LIBRARIES
        Qt5::Test
        KDev::Language
        KDev::Tests
        kdevqmljsduchain
)

########### next target ###############

set(testdeclarations_SRCS )

ecm_add_test(test_qmljsdeclarations.cpp
    LINK_LIBRARIES
        Qt5::Test
        KDev::Language
        KDev::Tests
        kdevqmljsduchain
)
