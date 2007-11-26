
include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}/util
)

#add_subdirectory(tests)

########### next target ###############

# FIXME remove
add_definitions(-DQT3_SUPPORT)
#-DQT3_SUPPORT_WARNINGS)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

set(kdevcppdebugger_PART_SRCS
    debuggerpart.cpp
    gdbcontroller.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    breakpoint.cpp
    variablewidget.cpp
    gdbbreakpointwidget.cpp
    framestackwidget.cpp
    disassemblewidget.cpp
    memviewdlg.cpp
    dbgpsdlg.cpp
    debuggerconfigwidget.cpp
    gdboutputwidget.cpp
    gdbtable.cpp
    debuggertracingdialog.cpp
    processlinemaker.cpp
    breakpointcontroller.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
)

set(kdevcppdebugger_PART_UI
    debuggerconfigwidget.ui
    debuggertracingdialog.ui
)

kde4_add_ui_files(kdevcppdebugger_PART_SRCS ${kdevcppdebugger_PART_UI})
kde4_add_plugin(kdevcppdebugger ${kdevcppdebugger_PART_SRCS})
target_link_libraries(kdevcppdebugger
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_EDITOR_LIBRARY}
    ${KDE4_KDE3SUPPORT_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
)

install(TARGETS kdevcppdebugger DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevcppdebugger.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevcppdebuggerui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcppdebugger )
