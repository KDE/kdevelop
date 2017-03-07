ecm_add_test(test_qmljscontexts.cpp
    LINK_LIBRARIES
        Qt5::Test
        KDev::Language
        KDev::Tests
        kdevqmljsduchain
)

ecm_add_test(test_qmljsdeclarations.cpp
    LINK_LIBRARIES
        Qt5::Test
        KDev::Language
        KDev::Tests
        kdevqmljsduchain
)
