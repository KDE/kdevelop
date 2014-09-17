configure_file(testqthelpconfig.h.in testqthelpconfig.h)

set(qthelpplugintest_SRCS
    testqthelpplugin.cpp
    ../qthelpplugin.cpp
    ../qthelpproviderabstract.cpp
    ../qthelpprovider.cpp
    ../qthelpdocumentation.cpp
    ../qthelpqtdoc.cpp
    ../qthelp_config_shared.cpp
)

add_executable(qthelpplugintest ${qthelpplugintest_SRCS} )
target_link_libraries( qthelpplugintest Qt5::Test KF5::KCMUtils KF5::KIOWidgets  KF5::TextEditor Qt5::Help Qt5::WebKitWidgets
    KDev::Language KDev::Documentation KDev::Interfaces KDev::Tests)

ecm_mark_as_test(qthelpplugintest)
add_test(qthelpplugintest qthelpplugintest)
