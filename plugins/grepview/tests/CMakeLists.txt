set(findReplaceTest_SRCS 
    test_findreplace.cpp
    ../grepviewplugin.cpp
    ../grepdialog.cpp
    ../grepoutputmodel.cpp
    ../grepoutputdelegate.cpp
    ../grepjob.cpp
    ../grepfindthread.cpp
    ../grepoutputview.cpp
    ../greputil.cpp
)
set(kdevgrepview_PART_UI
    ../grepwidget.ui
    ../grepoutputview.ui
)

qt5_wrap_ui(findReplaceTest_SRCS ${kdevgrepview_PART_UI})
ecm_add_test(${findReplaceTest_SRCS}
    TEST_NAME test_findreplace
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES}
    GUI)
