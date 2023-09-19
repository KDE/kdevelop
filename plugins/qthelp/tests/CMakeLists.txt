remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

configure_file(testqthelpconfig.h.in testqthelpconfig.h)

set(test_qthelpplugin_SRCS
    test_qthelpplugin.cpp
    ../qthelpplugin.cpp
    ../qthelpproviderabstract.cpp
    ../qthelpprovider.cpp
    ../qthelpdocumentation.cpp
    ../qthelpqtdoc.cpp
    ../qthelp_config_shared.cpp
    ../qthelpconfig.cpp
    ../qthelpnetwork.cpp
    ${kdevqthelp_LOG_SRCS}
)
ki18n_wrap_ui(test_qthelpplugin_SRCS
   ../qthelpconfig.ui
   ../qthelpconfigeditdialog.ui
)


ecm_add_test(${test_qthelpplugin_SRCS}
    TEST_NAME test_qthelpplugin
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF${KF_MAJOR_VERSION}::NewStuffWidgets KF${KF_MAJOR_VERSION}::KIOWidgets KF${KF_MAJOR_VERSION}::TextEditor KF${KF_MAJOR_VERSION}::IconThemes Qt${QT_MAJOR_VERSION}::Help KDev::Tests KDev::Documentation
)
