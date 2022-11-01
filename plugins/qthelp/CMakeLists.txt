add_definitions(-DTRANSLATION_DOMAIN=\"kdevqthelp\")

declare_qt_logging_category(kdevqthelp_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER QTHELP
    CATEGORY_BASENAME "qthelp"
)
set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
    qthelp_config_shared.cpp
    qthelpconfig.cpp # Configuration module for QtHelp plugin
    qthelpnetwork.cpp
    ${kdevqthelp_LOG_SRCS}
)

ki18n_wrap_ui(kdevqthelp_SRCS
    qthelpconfig.ui
    qthelpconfigeditdialog.ui
)

install(FILES kdevelop-qthelp.knsrc  DESTINATION ${KDE_INSTALL_KNSRCDIR})

kdevplatform_add_plugin(kdevqthelp SOURCES ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp
    KDev::Language
    KDev::Documentation
    KDev::Interfaces
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOWidgets
    KF5::TextEditor
    KF5::IconThemes
    KF5::NewStuffWidgets
    Qt5::Help
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
