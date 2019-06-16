find_package(OktetaGui CONFIG)
set_package_properties(OktetaGui PROPERTIES
    PURPOSE "Required for building MemoryViewer Widget."
    URL "https://kde.org/"
    TYPE OPTIONAL)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevgdb\")

set(kdevgdb_SRCS
    gdb.cpp
    gdboutputwidget.cpp
#    debuggertracingdialog.cpp
    gdbbreakpointcontroller.cpp
    debugsession.cpp
    variablecontroller.cpp
    gdbframestackmodel.cpp
    gdbvariable.cpp
    gdbconfigpage.cpp
)

if (OktetaGui_FOUND)
    set(KDEV_WITH_MEMVIEW true)
    list(APPEND kdevgdb_SRCS
        memviewdlg.cpp)
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config-gdb-plugin.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-gdb-plugin.h )

declare_qt_logging_category(kdevgdb_SRCS
    TYPE PLUGIN
    HEADER debuglog.h
    IDENTIFIER DEBUGGERGDB
    CATEGORY_BASENAME "gdb"
)
set(kdevgdb_UI
    debuggertracingdialog.ui
    gdbconfigpage.ui
)

kde_enable_exceptions()
ki18n_wrap_ui(kdevgdb_SRCS ${kdevgdb_UI})
qt5_add_resources(kdevgdb_SRCS kdevgdb.qrc)

# common code used by plugin and unit test
add_library(kdevgdb_static STATIC ${kdevgdb_SRCS})
target_link_libraries(kdevgdb_static
    kdevdebuggercommon
    KDev::Debugger
    KDev::Shell
    KDev::Interfaces
    KDev::IExecute
    KF5::KIOWidgets
    Qt5::Core
    Qt5::Gui
)
if(OktetaGui_FOUND)
    target_link_libraries(kdevgdb_static OktetaCore OktetaGui)
endif()
set_property(TARGET kdevgdb_static PROPERTY POSITION_INDEPENDENT_CODE ON)

# The actual plugin
kdevplatform_add_plugin(kdevgdb JSON kdevgdb.json SOURCES debuggerplugin.cpp )
target_link_libraries(kdevgdb kdevgdb_static)

if(BUILD_TESTING)
    add_subdirectory(unittests)
endif()
add_subdirectory(printers)
