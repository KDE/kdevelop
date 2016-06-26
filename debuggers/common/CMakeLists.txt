set(debuggercommon_SRCS
    mi/mi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
    mi/micommand.cpp
    mi/micommandqueue.cpp
    dialogs/selectcoredialog.cpp
    debuglog.cpp
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
)

find_package(KF5SysGuard)
if(KF5SysGuard_FOUND)
    add_definitions(-DKDEV_ENABLE_DBG_ATTACH_DIALOG)
    set(debuggercommon_SRCS
        ${debuggercommon_SRCS}
        dialogs/processselection.cpp
    )
endif()

ki18n_wrap_ui(debuggercommon_SRCS
    dialogs/selectcoredialog.ui
)

add_library(kdevdebuggercommon STATIC ${debuggercommon_SRCS})
target_link_libraries(kdevdebuggercommon
    PUBLIC
    KDev::Debugger
    PRIVATE
    Qt5::Core
    Qt5::Gui
    KDev::Util
    KDev::Language
)
if(KF5SysGuard_FOUND)
    target_link_libraries(kdevdebuggercommon
        PUBLIC
        KF5::ProcessUi
    )
endif()

kde_target_enable_exceptions(kdevdebuggercommon PUBLIC)
