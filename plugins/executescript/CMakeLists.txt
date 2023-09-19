add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecutescript\")

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)
declare_qt_logging_category(kdevexecutescript_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_EXECUTESCRIPT
    CATEGORY_BASENAME "executescript"
)
ki18n_wrap_ui(kdevexecutescript_PART_SRCS
    scriptappconfig.ui
)
kdevplatform_add_plugin(kdevexecutescript SOURCES ${kdevexecutescript_PART_SRCS})
target_link_libraries(kdevexecutescript
    KF6::KIOWidgets
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::Shell
)

install(FILES iexecutescriptplugin.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/executescript COMPONENT Devel)
