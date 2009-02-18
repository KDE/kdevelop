
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9012 )

include_directories(${CMAKE_SOURCE_DIR}/languages/cpp)
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
    util/treeitem.cpp
    util/treemodel.cpp
    util/treeview.cpp
    breakpointwidget.cpp
#    modeltest.cpp
    tooltipwidget.cpp
    ../../languages/cpp/stringhelpers.cpp
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
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBRARY}
)

install(TARGETS kdevgdb DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevgdb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevgdbui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgdb )
