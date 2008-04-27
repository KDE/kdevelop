
#remove cpp default area first
remove_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9012 )
include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}/util
    ${KDEVPLATFORM_INCLUDE_DIR}/
)

#add_subdirectory(tests)

########### next target ###############

set(kcm_kdev_cppdebugger_PART_SRCS
    debuggerconfigwidget.cpp
)

set(kcm_kdev_cppdebugger_PART_UI
    debuggerconfigwidget.ui
)

kde4_add_ui_files(kcm_kdev_cppdebugger_PART_SRCS ${kcm_kdev_cppdebugger_PART_UI})
kde4_add_kcfg_files( kcm_kdev_cppdebugger_PART_SRCS debuggerconfig.kcfgc )
kde4_add_plugin(kcm_kdev_cppdebugger ${kcm_kdev_cppdebugger_PART_SRCS})
target_link_libraries(kcm_kdev_cppdebugger
    ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS}
)

install(TARGETS kcm_kdev_cppdebugger DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_kdev_cppdebugger.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

set(kdevcppdebugger_PART_SRCS
    gdb.cpp    
    debuggerplugin.cpp
    gdbcontroller.cpp
    gdbcommandqueue.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    breakpoint.cpp
    variablewidget.cpp
#    abstractvariableitem.cpp
#    variableitem.cpp
#    frameitem.cpp
#    watchitem.cpp
    variablecollection.cpp
#    gdbbreakpointwidget.cpp
    framestackwidget.cpp
    disassemblewidget.cpp
    memviewdlg.cpp
    processselection.cpp
    gdboutputwidget.cpp
    debuggertracingdialog.cpp
    breakpointcontroller.cpp
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
#    qt4/qstringvariableitem.cpp
#    qt4/qlistvariableitem.cpp
    util/treeitem.cpp
    util/treemodel.cpp
    util/treeview.cpp
    breakpointwidget.cpp
#    modeltest.cpp
    tooltipwidget.cpp
    ../stringhelpers.cpp
)

set(kdevcppdebugger_PART_UI
    debuggertracingdialog.ui
    processselection.ui
)

kde4_add_ui_files(kdevcppdebugger_PART_SRCS ${kdevcppdebugger_PART_UI})
kde4_add_plugin(kdevcppdebugger ${kdevcppdebugger_PART_SRCS})
target_link_libraries(kdevcppdebugger sublime
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_EDITOR_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
)

install(TARGETS kdevcppdebugger DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevcppdebugger.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevcppdebuggerui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcppdebugger )
