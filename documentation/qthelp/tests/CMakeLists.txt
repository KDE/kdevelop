configure_file(testqthelpconfig.h.in testqthelpconfig.h)

set(test_qthelpplugin_SRCS
    test_qthelpplugin.cpp
    ../qthelpplugin.cpp
    ../qthelpproviderabstract.cpp
    ../qthelpprovider.cpp
    ../qthelpdocumentation.cpp
    ../qthelpqtdoc.cpp
    ../qthelp_config_shared.cpp
    ../debug.cpp
)

ecm_add_test(${test_qthelpplugin_SRCS}
    TEST_NAME test_qthelpplugin
    LINK_LIBRARIES Qt5::Test KF5::KCMUtils KF5::KIOWidgets  KF5::TextEditor Qt5::Help Qt5::WebKitWidgets
    KDev::Language KDev::Documentation KDev::Interfaces KDev::Tests)
