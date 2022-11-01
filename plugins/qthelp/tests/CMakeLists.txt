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
    LINK_LIBRARIES Qt5::Test KF5::NewStuffWidgets KF5::KIOWidgets KF5::TextEditor KF5::IconThemes Qt5::Help KDev::Tests KDev::Documentation
)
