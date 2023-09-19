add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

set(KDevPlatformDebugger_LIB_SRCS
    interfaces/idebugsession.cpp
    interfaces/iframestackmodel.cpp
    interfaces/ibreakpointcontroller.cpp
    interfaces/ivariablecontroller.cpp
    util/treeitem.cpp
    util/treemodel.cpp
    util/treeview.cpp
    util/pathmappings.cpp
    breakpoint/breakpoint.cpp
    breakpoint/breakpointmodel.cpp
    breakpoint/breakpointwidget.cpp
    breakpoint/breakpointdetails.cpp
    variable/variablewidget.cpp
    variable/variablecollection.cpp
    variable/variabletooltip.cpp
    variable/variablesortmodel.cpp
    framestack/framestackmodel.cpp
    framestack/framestackwidget.cpp
)
declare_qt_logging_category(KDevPlatformDebugger_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "debugger"
)
kdevplatform_add_library(KDevPlatformDebugger SOURCES ${KDevPlatformDebugger_LIB_SRCS})
target_link_libraries(KDevPlatformDebugger
PUBLIC
    KDev::Interfaces
    KDev::Util
PRIVATE
    KDev::Sublime
    KF6::Notifications
    KF6::TextEditor
)

install(FILES
    interfaces/idebugsession.h
    interfaces/ibreakpointcontroller.h
    interfaces/ivariablecontroller.h
    interfaces/iframestackmodel.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/debugger/interfaces COMPONENT Devel
)
install(FILES
    util/treemodel.h
    util/treeitem.h
    util/treeview.h
    util/pathmappings.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/debugger/util COMPONENT Devel
)
install(FILES
    breakpoint/breakpointwidget.h
    breakpoint/breakpointdetails.h
    breakpoint/breakpoint.h
    breakpoint/breakpointmodel.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/debugger/breakpoint COMPONENT Devel
)
install(FILES
    variable/variablecollection.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/debugger/variable COMPONENT Devel
)
install(FILES
    framestack/framestackmodel.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/debugger/framestack COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
