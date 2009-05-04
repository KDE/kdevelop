project(gdb)

add_subdirectory(unittests)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9012 )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    )
#add_subdirectory(tests)

#FIXME: remove me when in kdelibs
add_subdirectory(ksysguard)
include_directories(ksysguard/processcore)
#EoFIXME

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
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
#    qt4/qstringvariableitem.cpp
#    qt4/qlistvariableitem.cpp
#    modeltest.cpp
    tooltipwidget.cpp
    stringhelpers.cpp
    debugsession.cpp
    gdblaunchconfig.cpp
)

set(kdevgdb_PART_UI
    debuggertracingdialog.ui
    processselection.ui
    debuggerconfigwidget.ui
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
    kdev_processui #should change the name again when moved to kdelibs
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
    stackmanager.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
#    qt4/qstringvariableitem.cpp
#    qt4/qlistvariableitem.cpp
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
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBRARY}
    kdev_processui #to be removed when in kdelibs
)


########### install files ###############

install(FILES kdevgdb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevgdbui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgdb )
