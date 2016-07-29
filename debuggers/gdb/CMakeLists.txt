project(gdb)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevgdb\")

add_subdirectory(unittests)
add_subdirectory(printers)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

#add_subdirectory(tests)

########### next target ###############

set(kdevgdb_SRCS
    debuggerplugin.cpp
    gdb.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    debugsession.cpp
    gdbbreakpointcontroller.cpp
    gdbconfigpage.cpp
    variablecontroller.cpp
    gdbframestackmodel.cpp
    gdbvariable.cpp
)

if(OKTETA_FOUND)
    add_definitions(-DWITH_OKTETA=1)
    list(APPEND kdevgdb_SRCS
        memviewdlg.cpp)
endif()

set(kdevgdb_UI
    debuggertracingdialog.ui
    gdbconfigpage.ui
)

kde_enable_exceptions()
ki18n_wrap_ui(kdevgdb_SRCS ${kdevgdb_UI})
qt5_add_resources(kdevgdb_SRCS kdevgdb.qrc)
kdevplatform_add_plugin(kdevgdb JSON kdevgdb.json SOURCES ${kdevgdb_SRCS})
target_link_libraries(kdevgdb
    KDev::Sublime
    KDev::Interfaces
    KDev::Language
    KDev::Debugger
    KDev::OutputView
    KDev::Project
    KDev::Util
    KF5::TextEditor
    kdevdebuggercommon
)

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
        kdevdebuggercommon
)

if (HAVE_PATH_WITH_SPACES_TEST)
    set_target_properties(test_gdb PROPERTIES COMPILE_FLAGS "-DHAVE_PATH_WITH_SPACES_TEST")
endif()
