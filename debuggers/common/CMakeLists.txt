set(debuggercommon_SRCS
    mi/mi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
    mi/micommand.cpp
    mi/micommandqueue.cpp
)
#ki18n_wrap_ui(debuggercommon_SRCS something.ui)

add_library(kdevdebuggercommon STATIC ${debuggercommon_SRCS})
target_link_libraries(kdevdebuggercommon
    PRIVATE
    Qt5::Core
)

kde_target_enable_exceptions(kdevdebuggercommon PUBLIC)
