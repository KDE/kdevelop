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
    KF${KF_MAJOR_VERSION}::KCMUtils
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::TextEditor
    KF${KF_MAJOR_VERSION}::IconThemes
    KF${KF_MAJOR_VERSION}::NewStuffWidgets
    Qt${QT_MAJOR_VERSION}::Help
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
