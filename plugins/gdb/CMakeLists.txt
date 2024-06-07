# TODO: Okteta Qt6 port not yet complete, ETA 10/2024 or later.
# Once available, adapt code & reenable again. For now prevent finding the Qt5 version.
set(CMAKE_DISABLE_FIND_PACKAGE_OktetaGui ON)

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

# The actual plugin
set(kdevgdbplugin_SRCS
    debuggerplugin.cpp
    gdbconfigpage.cpp
)
set(kdevgdbplugin_UI
    gdbconfigpage.ui
)
ki18n_wrap_ui(kdevgdbplugin_SRCS ${kdevgdbplugin_UI})
kdevplatform_add_plugin(kdevgdb SOURCES ${kdevgdbplugin_SRCS})
target_link_libraries(kdevgdb kdevgdb_static)

if(BUILD_TESTING)
    add_subdirectory(unittests)
endif()
add_subdirectory(printers)
