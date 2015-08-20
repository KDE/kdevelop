include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/shell
    ${KDevPlatform_BINARY_DIR}/serialization
    )

set(TEST_PROBLEMSVIEW_SRC
    test_problemsview.cpp
    ../problemsview.cpp
    ../problemtreeview.cpp
    )

ecm_add_test(${TEST_PROBLEMSVIEW_SRC}
            TEST_NAME test_problemsview
            LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Shell
            )
