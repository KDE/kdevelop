project(gdb)

find_package(KDE4Workspace 4.2.0 REQUIRED)

add_subdirectory(unittests)
add_subdirectory(printers)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9012 )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4WORKSPACE_INCLUDE_DIR}
    )
#add_subdirectory(tests)

########### next target ###############

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

set(kdevgdb_PART_SRCS
    gdb.cpp
    debuggerplugin.cpp
    gdbcommandqueue.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    disassemblewidget.cpp
    memviewdlg.cpp
    processselection.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    breakpointcontroller.cpp
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
    stringhelpers.cpp
    debugsession.cpp
    gdblaunchconfig.cpp
    selectcoredialog.cpp
    variablecontroller.cpp
)

set(kdevgdb_PART_UI
    debuggertracingdialog.ui
    debuggerconfigwidget.ui
    selectcore.ui
)

kde4_add_ui_files(kdevgdb_PART_SRCS ${kdevgdb_PART_UI})
kde4_add_plugin(kdevgdb ${kdevgdb_PART_SRCS})
target_link_libraries(kdevgdb sublime
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDEVPLATFORM_DEBUGGER_LIBRARY}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4WORKSPACE_PROCESSUI_LIBS}
)

install(TARGETS kdevgdb DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(gdbtest_PART_SRCS
    unittests/gdbtest.cpp
    gdb.cpp
    gdbcommandqueue.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    disassemblewidget.cpp
    memviewdlg.cpp
    processselection.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    breakpointcontroller.cpp
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
    stringhelpers.cpp
    debugsession.cpp
    variablecontroller.cpp
)

kde4_add_ui_files(gdbtest_PART_SRCS
    debuggertracingdialog.ui
)
kde4_add_unit_test(gdbtest ${gdbtest_PART_SRCS})
target_link_libraries(gdbtest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_SHELL_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_DEBUGGER_LIBRARY}
    ${KDEVPLATFORM_TESTSHELL_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBRARY}
    ${KDE4WORKSPACE_PROCESSUI_LIBS}
)


########### install files ###############

install(FILES kdevgdb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevgdbui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgdb )
