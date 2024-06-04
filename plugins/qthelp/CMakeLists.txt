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
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOWidgets
    KF6::TextEditor
    KF6::IconWidgets
    KF6::NewStuffWidgets
    Qt::Help
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
