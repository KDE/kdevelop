

########### next target ###############

set( KDevPlatformDebugger_LIB_SRCS
    interfaces/idebugsession.cpp
    interfaces/iframestackmodel.cpp
    interfaces/ibreakpointcontroller.cpp
    interfaces/ivariablecontroller.cpp
    util/treeitem.cpp
    util/treemodel.cpp
    util/treeview.cpp
    util/pathmappings.cpp
    util/debug.cpp
    breakpoint/breakpoint.cpp
    breakpoint/breakpointmodel.cpp
    breakpoint/breakpointwidget.cpp
    breakpoint/breakpointdetails.cpp
    variable/variablewidget.cpp
    variable/variablecollection.cpp
    variable/variabletooltip.cpp
    framestack/framestackmodel.cpp
    framestack/framestackwidget.cpp
)
add_library(KDevPlatformDebugger ${KDevPlatformDebugger_LIB_SRCS})
generate_export_header(KDevPlatformDebugger EXPORT_FILE_NAME debuggerexport.h)
target_link_libraries(KDevPlatformDebugger LINK_PUBLIC
    KDev::Interfaces
    KDev::Util
LINK_PRIVATE
    Qt5::Core
    KF5::TextEditor
    KDev::Sublime
)
add_library(KDev::Debugger ALIAS KDevPlatformDebugger)

set_target_properties(KDevPlatformDebugger PROPERTIES
    VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION}
    EXPORT_NAME Debugger
)
install(TARGETS KDevPlatformDebugger EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/debuggerexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger COMPONENT Devel
)
install(FILES
    interfaces/idebugsession.h
    interfaces/ibreakpointcontroller.h
    interfaces/ivariablecontroller.h
    interfaces/iframestackmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/interfaces COMPONENT Devel
)
install(FILES
    util/treemodel.h
    util/treeitem.h
    util/treeview.h
    util/pathmappings.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/util COMPONENT Devel
)
install(FILES
    breakpoint/breakpointwidget.h
    breakpoint/breakpointdetails.h
    breakpoint/breakpoint.h
    breakpoint/breakpointmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/breakpoint COMPONENT Devel
)
install(FILES
    variable/variablecollection.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/variable COMPONENT Devel
)
install(FILES
    framestack/framestackmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/framestack COMPONENT Devel
)
