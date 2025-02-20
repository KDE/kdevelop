add_definitions(-DTRANSLATION_DOMAIN=\"kdevdebuggercommon\")
set(debuggercommon_SRCS
    mi/mi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
    mi/micommand.cpp
    mi/micommandqueue.cpp
    dbgglobal.cpp
    dialogs/selectcoredialog.cpp
    # debug session & debugger
    midebugger.cpp
    midebugsession.cpp
    midebuggerplugin.cpp
    midebugjobs.cpp
    # controllers
    mibreakpointcontroller.cpp
    miframestackmodel.cpp
    mivariablecontroller.cpp
    mivariable.cpp
    stringhelpers.cpp
    stty.cpp
    # tool views
    widgets/debuggerconsoleview.cpp
    widgets/disassemblewidget.cpp
    # register
    registers/registersview.cpp
    registers/registercontroller.cpp
    registers/registersmanager.cpp
    registers/registercontroller_x86.cpp
    registers/registercontroller_arm.cpp
    registers/modelsmanager.cpp
    registers/converters.cpp
)
if(KSysGuard_FOUND)
    list(APPEND debuggercommon_SRCS
        dialogs/processselection.cpp
    )
endif()

declare_qt_logging_category(debuggercommon_SRCS
    TYPE PLUGIN
    HEADER debuglog.h
    IDENTIFIER DEBUGGERCOMMON
    CATEGORY_BASENAME "debuggercommon"
)

ki18n_wrap_ui(debuggercommon_SRCS
    dialogs/processselection.ui
    dialogs/selectcoredialog.ui
    widgets/debuggerconsoleview.ui
    widgets/selectaddressdialog.ui
    registers/registersview.ui
)

add_library(kdevdebuggercommon STATIC ${debuggercommon_SRCS})
target_link_libraries(kdevdebuggercommon
    PUBLIC
    KDev::Debugger
    KDev::OutputView
    KDev::Sublime
    PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Gui
    Qt::Widgets
    KDev::Util
    KDev::Language
    KDev::IExecute
    KF6::KIOWidgets
)
target_include_directories(kdevdebuggercommon
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)
if(KSysGuard_FOUND)
    target_link_libraries(kdevdebuggercommon
        PUBLIC
            KSysGuard::ProcessCore
    )
endif()

kde_target_enable_exceptions(kdevdebuggercommon PUBLIC)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
