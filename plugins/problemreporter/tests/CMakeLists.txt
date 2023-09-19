set(TEST_PROBLEMSVIEW_SRC
    test_problemsview.cpp
    ../problemsview.cpp
    ../problemtreeview.cpp
    )
ecm_add_test(${TEST_PROBLEMSVIEW_SRC}
            TEST_NAME test_problemsview
            LINK_LIBRARIES Qt::Test KDev::Tests KDev::Shell KDev::Util
            )
