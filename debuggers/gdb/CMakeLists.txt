project(gdb)

add_subdirectory(unittests)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9012 )

find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${gdb_SOURCE_DIR}
    )
#add_subdirectory(tests)

########### next target ###############

set(kcm_kdev_gdb_PART_SRCS
    debuggerconfigwidget.cpp
)

set(kcm_kdev_gdb_PART_UI
    debuggerconfigwidget.ui
)

kde4_add_ui_files(kcm_kdev_gdb_PART_SRCS ${kcm_kdev_gdb_PART_UI})
kde4_add_kcfg_files( kcm_kdev_gdb_PART_SRCS debuggerconfig.kcfgc )
kde4_add_plugin(kcm_kdev_gdb ${kcm_kdev_gdb_PART_SRCS})
target_link_libraries(kcm_kdev_gdb
    ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS}
)

install(TARGETS kcm_kdev_gdb DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_kdev_gdb.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

set(kdevgdb_PART_SRCS
    gdb.cpp    
    debuggerplugin.cpp
    gdbcontroller.cpp
    gdbcommandqueue.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    variablewidget.cpp
#    abstractvariableitem.cpp
#    variableitem.cpp
#    frameitem.cpp
#    watchitem.cpp
    variablecollection.cpp
    disassemblewidget.cpp
    memviewdlg.cpp
    processselection.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    breakpointcontroller.cpp
    ibreakpointcontroller.cpp
    ibreakpoints.cpp
    inewbreakpoint.cpp
    breakpoints.cpp
    newbreakpoint.cpp
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
#    qt4/qstringvariableitem.cpp
#    qt4/qlistvariableitem.cpp
    breakpointwidget.cpp
#    modeltest.cpp
    tooltipwidget.cpp
    stringhelpers.cpp
    debugsession.cpp
)

set(kdevgdb_PART_UI
    debuggertracingdialog.ui
    processselection.ui
)

kde4_add_ui_files(kdevgdb_PART_SRCS ${kdevgdb_PART_UI})
kde4_add_plugin(kdevgdb ${kdevgdb_PART_SRCS})
target_link_libraries(kdevgdb sublime
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDEVPLATFORM_DEBUGGER_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBRARY}
)

install(TARGETS kdevgdb DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(gdbtest_PART_SRCS
    unittests/gdbtest.cpp
    gdb.cpp
    gdbcontroller.cpp
    gdbcommandqueue.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    variablewidget.cpp
#    abstractvariableitem.cpp
#    variableitem.cpp
#    frameitem.cpp
#    watchitem.cpp
    variablecollection.cpp
    disassemblewidget.cpp
    memviewdlg.cpp
    processselection.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    breakpointcontroller.cpp
    ibreakpointcontroller.cpp
    ibreakpoints.cpp
    inewbreakpoint.cpp
    breakpoints.cpp
    newbreakpoint.cpp
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
#    qt4/qstringvariableitem.cpp
#    qt4/qlistvariableitem.cpp
    breakpointwidget.cpp
#    modeltest.cpp
    tooltipwidget.cpp
    stringhelpers.cpp
    debugsession.cpp
)

kde4_add_ui_files(gdbtest_PART_SRCS
    debuggertracingdialog.ui
    processselection.ui
)
kde4_add_unit_test(gdbtest ${gdbtest_PART_SRCS})
target_link_libraries(gdbtest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_DEBUGGER_LIBRARY}
    ${KDEVPLATFORM_TESTSHELL_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBRARY}
)


########### install files ###############

install(FILES kdevgdb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevgdbui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgdb )
