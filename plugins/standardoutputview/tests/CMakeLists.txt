include_directories(
    ..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
set(test_standardOutputView_SRCS
    test_standardoutputview.cpp
    # TODO: this is partially broken:
    # these classes seem to be recompiled as their symbols are not exposed from the code in the plugin
    # a) results in that the compiler optimized the data structure differently,
    # b) results in separate staticMetaObject instances for the QObject subclasses, qobject_cast e.g. fails
    ../outputwidget.cpp
    ../outputwidgetconfig.cpp
    ../toolviewdata.cpp
    ../standardoutputview.cpp
    ${standardoutputview_LOG_PART_SRCS}
)

ecm_add_test(${test_standardOutputView_SRCS}
    TEST_NAME test_standardoutputview
    LINK_LIBRARIES Qt::Test KDev::Tests)
