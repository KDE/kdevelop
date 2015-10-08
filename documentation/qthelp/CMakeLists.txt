add_definitions(-DTRANSLATION_DOMAIN=\"kdevqthelp\")

set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
    qthelp_config_shared.cpp
    debug.cpp
    qthelpconfig.cpp # Configuration module for QtHelp plugin
)

ki18n_wrap_ui(kdevqthelp_SRCS
    qthelpconfig.ui
    qthelpconfigeditdialog.ui
)

kdevplatform_add_plugin(kdevqthelp JSON kdevqthelp.json SOURCES ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp
    KF5::KCMUtils KF5::I18n KF5::KIOWidgets KF5::TextEditor KF5::IconThemes Qt5::Help Qt5::WebKitWidgets KF5::NewStuff
    KDev::Language KDev::Documentation KDev::Interfaces)

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
install(FILES kdevelop-qthelp.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

add_subdirectory(tests)
