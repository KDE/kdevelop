set(findReplaceTest_SRCS 
    findreplacetest.cpp 
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
add_executable(findreplace-test ${findReplaceTest_SRCS})
add_test(findreplace-test findreplace-test)
ecm_mark_as_test(findreplace-test)
target_link_libraries(findreplace-test
    Qt5::Test
    
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)
