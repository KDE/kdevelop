remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(
    ..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
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
    ${kdevgrepview_LOG_PART_SRCS}
)
set(kdevgrepview_PART_UI
    ../grepwidget.ui
    ../grepoutputview.ui
)

ki18n_wrap_ui(findReplaceTest_SRCS ${kdevgrepview_PART_UI})
ecm_add_test(${findReplaceTest_SRCS}
    TEST_NAME test_findreplace
    LINK_LIBRARIES Qt::Test KDev::Language KDev::Project KDev::Util KDev::Tests KF6::Codecs KF6::TextWidgets KF6::KIOWidgets
    GUI)
