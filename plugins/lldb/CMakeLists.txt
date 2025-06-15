add_definitions(-DTRANSLATION_DOMAIN=\"kdevlldb\")

set(kdevlldb_SRCS
    lldbdebugger.cpp
    lldbcommand.cpp
    debugsession.cpp
    controllers/breakpointcontroller.cpp
    controllers/variablecontroller.cpp
    controllers/variable.cpp
    controllers/framestackmodel.cpp
    widgets/lldbconfigpage.cpp
    lldblauncher.cpp
)

declare_qt_logging_category(kdevlldb_SRCS
    TYPE PLUGIN
    HEADER debuglog.h
    IDENTIFIER DEBUGGERLLDB
    CATEGORY_BASENAME "lldb"
)

set(kdevlldb_UI
    widgets/lldbconfigpage.ui
)

ki18n_wrap_ui(kdevlldb_SRCS ${kdevlldb_UI})
qt_add_resources(kdevlldb_SRCS kdevlldb.qrc)

# common code used by plugin and unit test
add_library(kdevlldb_static STATIC ${kdevlldb_SRCS})
target_link_libraries(kdevlldb_static
    PUBLIC
    kdevdebuggercommon
    KDev::Debugger
    KDev::Interfaces
    KDev::IExecute
    PRIVATE
    Qt::Core
    Qt::Gui
    KF6::KIOWidgets
    KDev::Shell
)

# The actual plugin
kdevplatform_add_plugin(kdevlldb SOURCES debuggerplugin.cpp toolviewfactoryholder.cpp)
target_link_libraries(kdevlldb
    PUBLIC
    kdevlldb_static
    KDev::Interfaces
    KDev::Language
    KDev::Debugger
    KDev::OutputView
    KDev::Project
    KDev::Util
    KF6::TextEditor
    PRIVATE
    KDev::Sublime
)

# Data Formatters
add_subdirectory(formatters)

# Unit tests
if(BUILD_TESTING)
    add_subdirectory(unittests)
endif()
