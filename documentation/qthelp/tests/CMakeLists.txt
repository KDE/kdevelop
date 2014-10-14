configure_file(testqthelpconfig.h.in testqthelpconfig.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..) # "ui_qthelpconfig.h"
set(test_qthelpplugin_SRCS
    test_qthelpplugin.cpp
    ../qthelpplugin.cpp
    ../qthelpproviderabstract.cpp
    ../qthelpprovider.cpp
    ../qthelpdocumentation.cpp
    ../qthelpqtdoc.cpp
    ../qthelp_config_shared.cpp
    ../debug.cpp
    ../qthelpconfig.cpp
)

ecm_add_test(${test_qthelpplugin_SRCS}
    TEST_NAME test_qthelpplugin
    LINK_LIBRARIES Qt5::Test KF5::NewStuff KF5::KIOWidgets KF5::TextEditor KF5::IconThemes Qt5::Help Qt5::WebKitWidgets KDev::Tests KDev::Documentation
)
