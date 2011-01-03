
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

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

kde4_add_ui_files(findReplaceTest_SRCS ${kdevgrepview_PART_UI})
kde4_add_unit_test(findreplace-test ${findReplaceTest_SRCS})
target_link_libraries(findreplace-test
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)
