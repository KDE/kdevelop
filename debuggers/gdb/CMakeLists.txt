project(gdb)

# TODO KF5 figure out how to get the process selection widget (ksysguardprocesslist.h)
# macro_optional_find_package( KDE4Workspace 4.2.0 )

# macro_log_feature( KDE4WORKSPACE_FOUND
# "KDE 4 Workspace" "KDE 4 Workspace libraries" "kdebase/workspace" FALSE
# "" "Required for building the process selection dialog for attaching GDB to a running process."
# )

if(KDE4WORKSPACE_FOUND)
    include_directories(
        ${KDE4WORKSPACE_INCLUDE_DIR}
    )
    add_definitions(
        -DKDEV_ENABLE_GDB_ATTACH_DIALOG
    )
endif()

add_subdirectory(unittests)
add_subdirectory(printers)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

#add_subdirectory(tests)

########### next target ###############

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

set(kdevgdb_SRCS
    debuggerplugin.cpp
    gdb.cpp
    gdbcommandqueue.cpp
    gdbcommand.cpp
    gdbparser.cpp
    stty.cpp
    disassemblewidget.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    breakpointcontroller.cpp
    mi/gdbmi.cpp
    mi/milexer.cpp
    mi/miparser.cpp
    stringhelpers.cpp
    debugsession.cpp
    gdblaunchconfig.cpp
    debugjob.cpp
    selectcoredialog.cpp
    variablecontroller.cpp
    gdbframestackmodel.cpp
    gdbvariable.cpp
    registers/registersview.cpp
    registers/registercontroller.cpp
    registers/registersmanager.cpp
    registers/registercontroller_x86.cpp
    registers/registercontroller_arm.cpp
    registers/modelsmanager.cpp
    registers/converters.cpp
)

if(OKTETA_FOUND)
    list(APPEND kdevgdb_SRCS
        memviewdlg.cpp)
endif()


if(KDE4WORKSPACE_FOUND)
    set(kdevgdb_SRCS
        ${kdevgdb_SRCS}
        processselection.cpp
    )
endif(KDE4WORKSPACE_FOUND)

set(kdevgdb_UI
    debuggertracingdialog.ui
    debuggerconfigwidget.ui
    selectcore.ui
    selectaddress.ui
    registers/registersview.ui
)

qt5_wrap_ui(kdevgdb_SRCS ${kdevgdb_UI})
add_library(kdevgdb MODULE ${kdevgdb_SRCS})
target_link_libraries(kdevgdb
    KDev::Sublime
    KDev::Interfaces
    KDev::Language
    KDev::Debugger
    KDev::OutputView
    KDev::Project
    KDev::Util
    KF5::TextEditor
    ${KDE4WORKSPACE_PROCESSUI_LIBS}
)

install(TARGETS kdevgdb DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

set(test_gdb_SRCS unittests/test_gdb.cpp ${kdevgdb_SRCS})
ecm_add_test(${test_gdb_SRCS}
    TEST_NAME test_gdb
    LINK_LIBRARIES
        Qt5::Test
        KDev::Shell
        KDev::Interfaces
        KDev::Project
        KDev::Debugger
        KDev::Tests
        KDev::Util
        KF5::KIOWidgets
        KF5::TextEditor
        KF5::Parts
        ${KDE4WORKSPACE_PROCESSUI_LIBS}
)


########### install files ###############

configure_file(kdevgdb.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgdb.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevgdb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevgdbui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevgdb )
