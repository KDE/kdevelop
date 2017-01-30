project(gdb)

find_package(OktetaGui)
set_package_properties(OktetaGui PROPERTIES
    PURPOSE "Required for building MemoryViewer Widget."
    URL "http://kde.org/"
    TYPE OPTIONAL)

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

set(kdevtest_gdb_LINK_LIBS
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


if (OktetaGui_FOUND)
    set(KDEV_WITH_MEMVIEW true)
    list(APPEND kdevgdb_SRCS
        memviewdlg.cpp)
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-gdb-plugin.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-gdb-plugin.h )


set(kdevgdb_UI
    debuggertracingdialog.ui
    gdbconfigpage.ui
)

kde_enable_exceptions()
ki18n_wrap_ui(kdevgdb_SRCS ${kdevgdb_UI})
qt5_add_resources(kdevgdb_SRCS kdevgdb.qrc)
kdevplatform_add_plugin(kdevgdb JSON kdevgdb.json SOURCES ${kdevgdb_SRCS})

if(OktetaGui_FOUND)
    target_link_libraries(kdevgdb OktetaCore OktetaGui)
    list(APPEND kdevtest_gdb_LINK_LIBS
        OktetaCore OktetaGui)
endif()

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
    LINK_LIBRARIES ${kdevtest_gdb_LINK_LIBS}
)

if (HAVE_PATH_WITH_SPACES_TEST)
    set_target_properties(test_gdb PROPERTIES COMPILE_FLAGS "-DHAVE_PATH_WITH_SPACES_TEST")
endif()
